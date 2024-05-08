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

MASTER_PREFIX = "cf_0"
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
        self.master_position = PoseStamped()
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

        self.isMaster = cfname == MASTER_PREFIX

        self.goalTopic = self.create_subscription(
            Goal, f"{prefix}/ap_goal", self.ap_goal_callback, 10
        )

        self.abortTopic = self.create_subscription(
            Goal, f"{prefix}/ap_abort", self.ap_abort_callback, 10
        )

        # Master Branch
        if self.isMaster:
            self.goalStatePublisher = self.create_publisher(
                GoalFeedback, prefix + "/goal_state", qos_profile_services_default
            )

        self.cfPoseTopic = self.create_subscription(
            PoseStamped, f"{prefix}/pose", self.cf_pose_callback, 1
        )

        # self.takeoff(1.0, 1.0)

    def cf_pose_callback(self, msg:PoseStamped):
        pose = msg.pose.position
        self.cf_position = np.array([pose.x, pose.y, pose.z])

    def execute_trajectory(self, goal_vel, max_acceleration=1.0, max_speed=1.0):
        final_pos = self.cf_position + goal_vel

        # Calcolo dell'accelerazione
        # time_to_reach_final_velocity = np.linalg.norm(goal_vel) / max_acceleration
        # acceleration = goal_vel / time_to_reach_final_velocity
        acceleration = np.array([0.0, 0.0, 0.0])

        # Angolo di beccheggio e accelerazione angolare (presumibilmente zero in questo caso)
        yaw_angle = 0.0
        angular_velocity = np.array([0.0, 0.0, 0.0])

        self.cmdFullState(
            final_pos, goal_vel, acceleration, yaw_angle, angular_velocity
        )

    def execute_master_trajectory(self, msg: Goal):
        gf = GoalFeedback()
        gf.goal_state = GOAL_RUNNING
        gf.goal_id = msg.goal_id
        self.goalStatePublisher.publish(gf)

        traj = Trajectory()
        timeHelper = crazyflie.TimeHelper(self)
        share_folder = Path(__file__).parent
        traj.loadcsv(
            f"{share_folder}/../../../../share/crazyflie_controller/crazyflie_controller/trajectory_data/{msg.goal_id}.csv"
        )

        start_time = timeHelper.time()
        while not timeHelper.isShutdown():
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
        gf.goal_state = GOAL_REACHED
        gf.goal_id = msg.goal_id
        self.goalStatePublisher.publish(gf)
        self.notifySetpointsStop()
        self.land(0.03, 1.0)
        self.get_logger().info(f"Master GOAL ended")

    def ap_goal_callback(self, msg: Goal):
        if self.isMaster:
            self.get_logger().info(f"Received GOAL Msg")
            thread = threading.Thread(target=self.execute_master_trajectory, args=(msg,))
            thread.start()
        else:
            e_p = np.array([msg.x, msg.y, msg.qz])
            if not np.isnan(e_p).any():
                self.execute_trajectory(e_p)

    def ap_abort_callback(self, msg: Goal):
        self.get_logger().info(f"Received ABORT Msg")
        gf = GoalFeedback()
        gf.goal_state = GOAL_ABORTED
        gf.goal_id = msg.goal_id
        self.goalStatePublisher.publish(gf)
        self.notifySetpointsStop()
        self.land(0.03, 1.0)

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
        q = rowan.from_euler(0, 0, yaw)
        self.cmdFullStateMsg.pose.orientation.w = q[0]
        self.cmdFullStateMsg.pose.orientation.x = q[1]
        self.cmdFullStateMsg.pose.orientation.y = q[2]
        self.cmdFullStateMsg.pose.orientation.z = q[3]
        self.cmdFullStateMsg.twist.angular.x = omega[0]
        self.cmdFullStateMsg.twist.angular.y = omega[1]
        self.cmdFullStateMsg.twist.angular.z = omega[2]
        self.cmdFullStatePublisher.publish(self.cmdFullStateMsg)

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
