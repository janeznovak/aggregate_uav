#!/usr/bin/env python
import sys
import rclpy
from rclpy.node import Node
import numpy as np
import rowan
from pathlib import Path

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
        self.cf_position = PoseStamped()

        self.takeoffService = self.create_client(Takeoff, prefix + "/takeoff")
        self.takeoffService.wait_for_service()
        self.landService = self.create_client(Land, prefix + "/land")
        self.landService.wait_for_service()

        self.cmdFullStatePublisher = self.create_publisher(
            FullState, prefix + "/cmd_full_state", 1
        )
        self.cmdFullStateMsg = FullState()
        self.cmdFullStateMsg.header.frame_id = "/world"

        self.cmdPositionPublisher = self.create_publisher(
            Position, prefix + "/cmd_position", 1
        )
        self.cmdPositionMsg = Position()
        self.cmdPositionMsg.header.frame_id = "/world"

        self.isMaster = cfname == MASTER_PREFIX

        self.goalTopic = self.create_subscription(
            Goal, f"{prefix}/ap_goal", self.ap_goal_callback, 10
        )

        self.goalTopic = self.create_subscription(
            Goal, f"{prefix}/ap_abort", self.ap_abort_callback, 10
        )

        # Master Branch
        if self.isMaster:
            self.goalStatePublisher = self.create_publisher(
                GoalFeedback, prefix + "/goal_state", qos_profile_services_default
            )

        # TODO: Testing Section. Add Trajectory
        # self.takeoff(1.0, 1.0)

        # self.cfPoseTopic = self.create_subscription(
        #     PoseStamped, f"{prefix}/pose", self.cf_pose_callback, 1
        # )

        # # Distance/angle
        # self.distance = np.array([2.0, 0.0, 0.0])  # Distanza desiderata
        # self.angle = np.pi / 2

        # # Slave code
        # if not self.isMaster:
        #     self.masterPoseTopic = self.create_subscription(
        #         PoseStamped, f"/{MASTER_PREFIX}/pose", self.master_pose_callback, 1
        #     )

    def execute_trajectory(self, goal_vel, max_acceleration=1.0, max_speed=1.0):
        goal_velocity = goal_vel

        # Calcolo dell'accelerazione
        time_to_reach_final_velocity = np.linalg.norm(goal_velocity) / max_acceleration
        acceleration = goal_velocity / time_to_reach_final_velocity

        # Angolo di beccheggio e accelerazione angolare (presumibilmente zero in questo caso)
        yaw_angle = 0.0
        angular_velocity = np.array([0.0, 0.0, 0.0])

        self.cmdFullState(
            goal_velocity, goal_velocity, acceleration, yaw_angle, angular_velocity
        )

    def execute_master_trajectory(self, trajpath, rate=100, offset=np.zeros(3)):
        traj = Trajectory()
        timeHelper = crazyflie.TimeHelper(self)
        share_folder = Path(__file__).parent
        traj.loadcsv(
            f"{share_folder}/../../../../share/crazyflie_controller/crazyflie_controller/trajectory_data/{trajpath}.csv"
        )

        start_time = timeHelper.time()
        while not timeHelper.isShutdown():
            t = timeHelper.time() - start_time
            if t > traj.duration:
                break

            e = traj.eval(t)

            self.cmdFullState(
                e.pos + offset,
                e.vel,
                e.acc,
                e.yaw,
                e.omega,
            )

        timeHelper.sleepForRate(rate)

    def ap_goal_callback(self, msg: Goal):
        self.get_logger().info(f"Received GOAL Msg")
        gf = GoalFeedback()
        gf.goal_state = GOAL_RUNNING
        gf.goal_id = msg.goal_id
        self.goalStatePublisher.publish(gf)

        if self.isMaster:
            self.execute_master_trajectory(msg.goal_id)
            gf = GoalFeedback()
            self.get_logger().info(f"Master GOAL ended")
            gf.goal_state = GOAL_REACHED
            gf.goal_id = msg.goal_id
            self.goalStatePublisher.publish(gf)
        else:
            e_p = np.array([msg.x, msg.y, msg.qz])
            self.execute_trajectory(e_p)

    def ap_abort_callback(self, msg: Goal):
        self.get_logger().info(f"Received ABORT Msg")
        gf = GoalFeedback()
        gf.goal_state = GOAL_ABORTED
        gf.goal_id = msg.goal_id
        self.goalStatePublisher.publish(gf)
        self.land(0.0, 1.0)

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

    def cmdPosition(self, pos, yaw=0.0):
        self.cmdPositionMsg.header.stamp = self.node.get_clock().now().to_msg()
        self.cmdPositionMsg.x = pos[0]
        self.cmdPositionMsg.y = pos[1]
        self.cmdPositionMsg.z = pos[2]
        self.cmdPositionMsg.yaw = yaw
        self.cmdPositionPublisher.publish(self.cmdPositionMsg)


def main(args=None):
    rclpy.init()
    cfname = sys.argv[1]
    node = CrazyflyController(cfname)
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
