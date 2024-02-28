# Copyright 2016 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
from rclpy.duration import Duration
from threading import Lock, Thread

from std_msgs.msg import Float32
from nav_msgs.msg import Odometry
from geometry_msgs.msg import PoseStamped
from irobot_create_msgs.action import Undock
from irobot_create_msgs.action import Dock
from battery_services.srv import SetCharge
from nav_system_interfaces.msg import Goal, GoalFeedback
from nav2_simple_commander.robot_navigator import BasicNavigator, TaskResult

GOAL_REACHED: int  = 0
GOAL_ABORTED : int = 1
GOAL_FAILED : int  = 2
GOAL_RUNNING : int = 3
GOAL_UNKNOWN : int = 4
GOAL_NONE : int    = -1

class PocNavigator(Node):
    """!
    Class used for issuing navigation goal using NAV2.
    """

    x: float
    y: float
    w: float
    nav: BasicNavigator
    navigating: bool

    def __init__(self, namespace, nav):
        """!
        PocNavigator constructor requires a namespace for topics and a
        nav2_simple_commander.robot_navigator.BasicNavigator node.
        Declares the node parameter decrease_battery to enable battery level
        decrease whenever a navigation goal is reached.
        """
        super().__init__('poc_navigator', namespace=namespace)
        self.declare_parameter('decrease_battery', False)
        self.goal_subscription = self.create_subscription(
            Goal,
            'ap_goal',
            self.new_goal,
            10)
        self.abort_subscription = self.create_subscription(
            Goal,
            'ap_abort',
            self.abort_goal,
            10)
        self.set_charge_battery_srv = self.create_client(SetCharge, 'gazebo_ros_battery/set_charge')
        self.dock_action = ActionClient(self, Dock, 'dock')
        self.undock_action = ActionClient(self, Undock, 'undock')
        self.timer = self.create_timer(1.0, self.check_navigation_status)
        self.publisher_ = self.create_publisher(GoalFeedback, 'goal_state', 10)
        self.goal_subscription  # prevent unused variable warning
        self.abort_subscription  # prevent unused variable warning
        self.current_goal_id = ""
        self.nav = nav
        self.navigating = False
        msg_nav2 = GoalFeedback()
        msg_nav2.goal_id = self.current_goal_id
        msg_nav2.goal_state = GOAL_NONE
        self.publisher_.publish(msg_nav2)

    def get_coordinates(self):
        return {'x': self.x, 'y': self.y, 'w': self.w}

    def new_goal(self, msg):
        """!
        Given a nav_system_interfaces.msg.Goal message sends the robot to
        the requested coordinates or executes a dock/undock action.
        """
        # TODO: add management to SOS goal type, at the moment it's the same as GOAL
        if msg.type == 'GOAL' or msg.type == 'SOS':
            # Wait for self.navigation to fully activate, since autostarting nav2
            self.nav.waitUntilNav2Active()
            self.current_goal_id = msg.goal_id

            # Set goal pose
            goal_pose = PoseStamped()
            goal_pose.header.frame_id = 'map'
            goal_pose.header.stamp = self.nav.get_clock().now().to_msg()
            goal_pose.pose.position.x = msg.x
            goal_pose.pose.position.y = msg.y
            goal_pose.pose.orientation.z = msg.qz
            goal_pose.pose.orientation.w = msg.qw
            # sanity check a valid path exists
            # path = self.nav.getPath(initial_pose, goal_pose)

            self.get_logger().info('Publishing: "%s"' % goal_pose)
            self.nav.goToPose(goal_pose)
            msg_nav2 = GoalFeedback()
            msg_nav2.goal_id = self.current_goal_id
            msg_nav2.goal_state = GOAL_RUNNING
            self.publisher_.publish(msg_nav2)
            self.navigating = True
        if msg.type == 'DOCK':
            self.get_logger().info('Requesting dock action')
            self.dock_action.send_goal_async(Dock.Goal())
        if msg.type == 'UNDOCK':
            self.get_logger().info('"Requesting undock action')
            self.undock_action.send_goal_async(Undock.Goal())

    def decrease_battery(self):
        """!
        Decrease the battery level of -0.1 units.
        Executed if decrease_battery is True.
        """
        while not self.set_charge_battery_srv.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('service not available, waiting again...')
        self.req = SetCharge.Request()
        self.req.charge = Float32()
        self.req.charge.data = -0.1
        self.future = self.set_charge_battery_srv.call_async(self.req)
        self.get_logger().info('Decreasing battery after reaching goal.')

    def check_navigation_status(self):
        """!
        Verify current navigation state, publishing the goal_state messages
        accordingly.
        Called once per second.
        """
        if self.navigating and self.nav.isTaskComplete():
            result = self.nav.getResult()
            msg = GoalFeedback()
            msg.goal_id = self.current_goal_id
            if result == TaskResult.SUCCEEDED:
                self.get_logger().info('Goal succeeded!')
                msg.goal_state = GOAL_REACHED
                if self.get_parameter('decrease_battery').get_parameter_value().bool_value:
                    self.decrease_battery()
                self.publisher_.publish(msg)

            elif result == TaskResult.CANCELED:
                self.get_logger().info('Goal was canceled!')
                msg.goal_state = GOAL_ABORTED
                self.publisher_.publish(msg)
            elif result == TaskResult.FAILED:
                self.get_logger().info('Goal failed!')
                msg.goal_state = GOAL_FAILED
                self.publisher_.publish(msg)
            else:
                self.get_logger().info('Goal has an invalid return status!')
                msg.goal_state = GOAL_UNKNOWN
                self.publisher_.publish(msg)

            self.navigating = False

    def abort_goal(self, msg):
        """!
        Cancel the current goal.
        Subscribed to ap_abort.
        """
        self.get_logger().info('Aborting goal: "%s"' % self.nav)
        self.nav.cancelTask()

def main(args=sys.argv):
    rclpy.init(args=args)
    navigator = BasicNavigator(namespace=args[1])
    poc_navigator = PocNavigator(namespace=args[1], nav=navigator)
    rclpy.spin(poc_navigator)
    poc_navigator.destroy_node()
    rclpy.shutdown()
    exit(0)

if __name__ == '__main__':
    main()
