#!/usr/bin/env python
import sys
import rclpy
from rclpy.node import Node
import numpy as np
import rowan
from pathlib import Path
import numpy as np
import threading

from crazyflie_interfaces.msg import FullState, Position
from crazyflie_interfaces.srv import Land, NotifySetpointsStop, Takeoff
from geometry_msgs.msg import PoseStamped
from nav_system_interfaces.msg import Goal
from nav_system_interfaces.msg import GoalFeedback
from crazyflie_py.uav_trajectory import Trajectory
from crazyflie_py import crazyflie
from rclpy.qos import qos_profile_services_default

MASTER_PREFIXES = ["cf_0", "cf_1"]
GOAL_REACHED: int = 0
GOAL_ABORTED: int = 1
GOAL_FAILED: int = 2
GOAL_RUNNING: int = 3
GOAL_UNKNOWN: int = 4
GOAL_NONE: int = -1


class CrazyflyController(Node):
    def __init__(self, cfname):
        super().__init__("controller", namespace="/" + cfname)
        self.get_logger().info("Spawn complete Node: " + cfname)

        prefix = "/" + cfname
        self.cfname = cfname
        self.prefix = prefix

        # self.isMaster = cfname == MASTER_PREFIX
        self.isMaster = cfname in MASTER_PREFIXES
        self.cf_position = np.array([0.0, 0.0, 0.0])

        self.takeoffService = self.create_client(Takeoff, prefix + "/takeoff")
        self.takeoffService.wait_for_service()
        self.landService = self.create_client(Land, prefix + "/land")
        self.landService.wait_for_service()

        self.notifySetpointsStopService = self.create_client(
            NotifySetpointsStop, prefix + '/notify_setpoints_stop')
        self.notifySetpointsStopService.wait_for_service()

        self.cmdFullStatePublisher = self.create_publisher(
            FullState, prefix + "/cmd_full_state", 1
        )
        self.cmdFullStateMsg = FullState()
        self.cmdFullStateMsg.header.frame_id = "/world"

        self.goalTopic = self.create_subscription(
            Goal, f"{prefix}/ap_goal", self.ap_goal_callback, 10
        )

        self.cfPoseTopic = self.create_subscription(
            PoseStamped, f"{prefix}/pose", self.cf_pose_callback, 1
        )

        # Master Branch
        if self.isMaster:
            self.abort_flag = False
            self.goalStatePublisher = self.create_publisher(
                GoalFeedback, prefix + "/goal_state", qos_profile_services_default
            )

            self.abortTopic = self.create_subscription(
                Goal, f"{prefix}/ap_abort", self.ap_abort_callback, 10
            )

    def cf_pose_callback(self, msg:PoseStamped):
        pose = msg.pose.position
        self.cf_position = np.array([pose.x, pose.y, pose.z])

    def execute_trajectory(self, goal_pos, max_acceleration=1.0, max_speed=1.0):
        final_pos = self.cf_position + goal_pos

        # Calcolo dell'accelerazione
        # time_to_reach_final_velocity = np.linalg.norm(goal_vel) / max_acceleration
        # acceleration = goal_vel / time_to_reach_final_velocity
        acceleration = np.array([0.0, 0.0, 0.0])

        # Angolo di beccheggio e accelerazione angolare (presumibilmente zero in questo caso)
        yaw_angle = 0.0
        angular_velocity = np.array([0.0, 0.0, 0.0])
        
        # self.cmdPosition(final_pos, yaw_angle)

        self.cmdFullState(
            final_pos, goal_pos, acceleration, yaw_angle, angular_velocity
        )

    def execute_master_trajectory(self, msg: Goal):
        gf = GoalFeedback()
        gf.goal_state = GOAL_RUNNING
        gf.goal_id = msg.goal_id
        self.goalStatePublisher.publish(gf)

        traj = Trajectory()
        timeHelper = crazyflie.TimeHelper(self)
        share_folder = Path(__file__).parent
        traj_name = msg.goal_id.split("-")[0]
        master_id = msg.goal_id.split("-")[2]
        robot_real_id = self.cfname.split("_")[1]
        
        if master_id == robot_real_id:
            self.get_logger().info(f"The master id is: {master_id} and the robot real name is: {robot_real_id}")
            traj.loadcsv(
                f"{share_folder}/trajectory_data/{traj_name}.csv"
            )

        start_time = timeHelper.time()
        while not timeHelper.isShutdown() and not self.abort_flag:
            t = timeHelper.time() - start_time
            if t > traj.duration:
                break

            e = traj.eval(t)
            self.cmdFullState(
                e.pos,
                e.vel,
                e.acc,
                e.yaw,
                e.omega,
            )

        # Goal Ended
        if not self.abort_flag:
            gf.goal_state = GOAL_REACHED
            gf.goal_id = msg.goal_id
            self.goalStatePublisher.publish(gf)
            self.get_logger().info(f"Master GOAL ended")
        else:
            gf.goal_state = GOAL_ABORTED
            gf.goal_id = msg.goal_id
            self.goalStatePublisher.publish(gf)
            self.get_logger().info(f"Master GOAL aborted")

        self.notifySetpointsStop()
        self.land(0.15, 10.0)

    def ap_goal_callback(self, msg: Goal):
        if self.isMaster:
            master_id = msg.goal_id.split("-")[2]
            robot_real_id = self.cfname.split("_")[1]
            self.get_logger().info(f"The master id is: {master_id} and the robot real name is: {robot_real_id}")
            if msg.type == "GOAL" and master_id == robot_real_id:
                self.get_logger().info(f"This is the message: {msg}")
                self.abort_flag = False
                self.get_logger().info(f"Received GOAL Msg")
                thread = threading.Thread(target=self.execute_master_trajectory, args=(msg,))
                thread.start()
        else:
            if msg.type == "LAND":
                self.get_logger().info("Landing Message SLAVE")
                self.notifySetpointsStop()
                self.land(0.15, 10.0)
            else:
                e_p = np.array([msg.x, msg.y, msg.qz])
                if not np.isnan(e_p).any():
                    self.execute_trajectory(e_p)

    def ap_abort_callback(self, msg: Goal):
        self.get_logger().info(f"Received ABORT Msg")
        self.abort_flag = True

    def takeoff(self, targetHeight, duration, groupMask=0):
        req = Takeoff.Request()
        req.group_mask = groupMask
        req.height = targetHeight
        req.duration = rclpy.duration.Duration(seconds=duration).to_msg()
        self.takeoffService.call_async(req)

    def land(self, targetHeight, duration, groupMask=0):
        """
        Execute a landing - fly straight down. User must cut power after.

        Asynchronous command; returns immediately.

        Args:
        ----
            targetHeight (float): The z-coordinate at which to land. Meters.
                Usually should be a few centimeters above the initial position
                to ensure that the controller does not try to penetrate the
                floor if the mocap coordinate origin is not perfect.
            duration (float): How long until the height is reached. Seconds.
            groupMask (int): Group mask bits. See :meth:`setGroupMask()` doc.

        """
        req = Land.Request()
        req.group_mask = groupMask
        req.height = targetHeight
        req.duration = rclpy.duration.Duration(seconds=duration).to_msg()
        self.landService.call_async(req)

    def cmdFullState(self, pos, vel, acc, yaw, omega):
        """
        Args:
            pos (array-like of float[3]): Position. Meters.
            vel (array-like of float[3]): Velocity. Meters / second.
            acc (array-like of float[3]): Acceleration. Meters / second^2.
            yaw (float): Yaw angle. Radians.
            omega (array-like of float[3]): Angular velocity in body frame.
                Radians / sec.
        """
        self.cmdFullStateMsg.header.stamp = self.get_clock().now().to_msg()
        self.cmdFullStateMsg.pose.position.x = pos[0]
        self.cmdFullStateMsg.pose.position.y = pos[1]
        self.cmdFullStateMsg.pose.position.z = pos[2]
        self.cmdFullStateMsg.twist.linear.x = vel[0]
        self.cmdFullStateMsg.twist.linear.y = vel[1]
        self.cmdFullStateMsg.twist.linear.z = vel[2]
        self.cmdFullStateMsg.acc.x = acc[0]
        self.cmdFullStateMsg.acc.y = acc[1]
        self.cmdFullStateMsg.acc.z = acc[2]
        q = rowan.from_euler(0, yaw, 0)
        self.cmdFullStateMsg.pose.orientation.w = q[0]
        self.cmdFullStateMsg.pose.orientation.x = q[1]
        self.cmdFullStateMsg.pose.orientation.y = q[2]
        self.cmdFullStateMsg.pose.orientation.z = q[3]
        self.cmdFullStateMsg.twist.angular.x = omega[0]
        self.cmdFullStateMsg.twist.angular.y = omega[1]
        self.cmdFullStateMsg.twist.angular.z = omega[2]
        self.cmdFullStatePublisher.publish(self.cmdFullStateMsg)
        
    def cmdPosition(self, pos, yaw=0.):
        """
        Send a streaming command of absolute position and yaw setpoint.

        Useful for slow maneuvers where a high-level planner determines the
        desired position, and the rest is left to the onboard controller.

        For more information on streaming setpoint commands, see the
        :meth:`cmdFullState()` documentation.

        Args:
        ----
            pos (array-like of float[3]): Position. Meters.
            yaw (float): Yaw angle. Radians.

        """
        self.cmdPositionMsg.header.stamp = self.get_clock().now().to_msg()
        self.cmdPositionMsg.x = pos[0]
        self.cmdPositionMsg.y = pos[1]
        self.cmdPositionMsg.z = pos[2]
        self.cmdPositionMsg.yaw = yaw
        self.cmdPositionPublisher.publish(self.cmdPositionMsg)
        
        self.get_logger().info(f"Position: {pos} - Yaw: {yaw}")
        
    def goTo(self, goal, yaw, duration, relative=False, groupMask=0):
        """
        Move smoothly to the goal, then hover indefinitely.

        Asynchronous command; returns immediately.

        Plans a smooth trajectory from the current state to the goal position.
        Will stop smoothly at the goal with minimal overshoot. If the current
        state is at hover, the planned trajectory will be a straight line;
        however, if the current velocity is nonzero, the planned trajectory
        will be a smooth curve.

        Plans the trajectory by solving for the unique degree-7 polynomial that
        satisfies the initial conditions of the current position, velocity,
        and acceleration, and ends at the goal position with zero velocity and
        acceleration. The jerk (derivative of acceleration) is fixed at zero at
        both boundary conditions.

        Note: it is the user's responsibility to ensure that the goTo command
        is feasible. If the duration is too short, the trajectory will require
        impossible accelerations and velocities. The planner will not correct
        this, and the failure to achieve the desired states will cause the
        controller to become unstable.

        Args:
            goal (iterable of 3 floats): The goal position. Meters.
            yaw (float): The goal yaw angle (heading). Radians.
            duration (float): How long until the goal is reached. Seconds.
            relative (bool): If true, the goal position is interpreted as a
                relative offset from the current position. Otherwise, the goal
                position is interpreted as absolute coordintates in the global
                reference frame.
            groupMask (int): Group mask bits. See :meth:`setGroupMask()` doc.

        """
        req = GoTo.Request()
        req.group_mask = groupMask
        req.relative = relative
        req.goal = arrayToGeometryPoint(goal)
        req.yaw = float(yaw)
        req.duration = rclpy.duration.Duration(seconds=duration).to_msg()
        self.goToService.call_async(req)

    def notifySetpointsStop(self, remainValidMillisecs=100, groupMask=0):
        """
        Informs that streaming low-level setpoint packets are about to stop.

        Streaming setpoints are :meth:`cmdVelocityWorld`, :meth:`cmdFullState`,
        and so on. For safety purposes, they normally preempt onboard high-level
        commands such as :meth:`goTo`.

        Once preempted, the Crazyflie will not switch back to high-level
        commands (or other behaviors determined by onboard planning/logic) until
        a significant amount of time has elapsed where no low-level setpoint
        was received.

        This command short-circuits that waiting period to a user-chosen time.
        It should be called after sending the last low-level setpoint, and
        before sending any high-level command.

        A common use case is to execute the :meth:`land` command after using
        streaming setpoint modes.

        Args:
            remainValidMillisecs (int): Number of milliseconds that the last
                streaming setpoint should be followed before reverting to the
                onboard-determined behavior. May be longer e.g. if one radio
                is controlling many robots.
            groupMask (int): Group mask bits. See :meth:`setGroupMask()` doc.

        """
        req = NotifySetpointsStop.Request()
        req.remain_valid_millisecs = remainValidMillisecs
        req.group_mask = groupMask
        self.notifySetpointsStopService.call_async(req)


def main(args=None):
    rclpy.init()
    cfname = sys.argv[1]
    node = CrazyflyController(cfname)
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
