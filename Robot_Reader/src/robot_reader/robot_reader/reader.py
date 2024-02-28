# Copyright © 2023 University of Turin, Daniele Bortoluzzi & Giordano Scarso. All Rights Reserved.

import sys
import rclpy
from rclpy.node import Node
from rclpy.executors import MultiThreadedExecutor

from std_msgs.msg import String
from nav_msgs.msg import Odometry
from nav_system_interfaces.msg import GoalFeedback
from sensor_msgs.msg import BatteryState
from geometry_msgs.msg import PoseWithCovarianceStamped
from rclpy.qos import *

import time, threading, string

from . import store
from . import config
from . import coords
from .position import PositionDTO
from .battery import BatteryDTO
from .goal import GoalDTO
from .feedback_writer import FeedbackWriter

class RobotInformationReader(Node):
    """!
    Class exteding ROS2 Node to receive sensor data from the robot and
    write it onto files to be used by other programs.
    """

    def __init__(self, namespace, robot_name, origin_x=0.0, origin_y=0.0, rotation=0.0):
        """!
        Class' constructor requiring a namespace and optionally the robot
        coordinates relative to the origin.
        """
        super().__init__('robot_information_reader', namespace=namespace)
        self.robot_name = robot_name
        self.origin_x = float(origin_x)
        self.origin_y = float(origin_y)
        self.rotation = float(rotation)
        self.timer = self.create_timer(config.POLL_WRITER_SECONDS, self.write_file)
        self.feedbackWriter = FeedbackWriter()
        self.get_logger().info('Robot reader initialized')
        self.subscription_position = self.position_subscribe(namespace)

        # subscribe battery
        self.get_logger().info('Subscribing to: "%s/%s"' % (namespace, config.BATTERY_TOPIC))
        self.subscription_battery = self.create_subscription(
            BatteryState,
            config.BATTERY_TOPIC,
            self.listener_battery_callback,
            config.BATTERY_QOS_TOPIC)

        # subscribe goal
        self.get_logger().info('Subscribing to: "%s/%s"' % (namespace, config.GOAL_TOPIC))
        self.subscription_goal = self.create_subscription(
            GoalFeedback,
            config.GOAL_TOPIC,
            self.listener_goal_callback,
            config.DEFAULT_QOS_TOPIC)
        self.subscription_position
        self.subscription_battery
        self.subscription_goal

    def position_subscribe(self, namespace):
        # subscribe amcl
        self.get_logger().info('Subscribing to: "%s/%s"' % (namespace, config.AMCL_TOPIC))
        # durability = 1 == TRANSIENT_LOCAL
        return self.create_subscription(
                PoseWithCovarianceStamped,
                config.AMCL_TOPIC,
                self.listener_position_callback,
                QoSProfile(durability=1, depth=5)
                )

    def listener_position_callback(self, msg):
        """!
        Given a message with pose.pose.position, converts the coordinate
        changing the origin point from the robot origin point to the global
        origin point. Callback for the position topic.
        """
        x, y, angle = coords.rel2abs(msg.pose.pose.position.x, msg.pose.pose.position.y,
                msg.pose.pose.orientation.z, msg.pose.pose.orientation.w,
                self.origin_x, self.origin_y, self.rotation)
        position            = PositionDTO(self.robot_name, x, y, angle)
        store.content._dictionary[self.robot_name]["position"]     = position
        self.get_logger().debug('I heard position from %s: "%s"' % (self.robot_name, str(position)))

    def listener_battery_callback(self, msg: BatteryState):
        """!
        Given a sensor_msgs.msg.BatteryState message it stores it into
        the content dictionary. Callback for the /battery_state topic.
        """
        battery = BatteryDTO(self.robot_name,
                # range of msg.percentage is different for each robots.
                # so we compute the percentage
                msg.charge/msg.capacity*100,
                msg.temperature,
                msg.capacity,
                msg.power_supply_status)
        store.content._dictionary[self.robot_name]["battery"] = battery
        self.get_logger().debug('I heard battery from %s: "%s"' % (self.robot_name, str(battery)))

    def listener_goal_callback(self, msg: GoalDTO):
        """!
        Given a Goal message it stores it into the content dictionary.
        Callback for the /goal_state topic.
        """
        goal = GoalDTO(self.robot_name, msg.goal_id, msg.goal_state)
        store.content._dictionary[self.robot_name]["goal"]         = goal
        self.get_logger().debug('I heard goal from %s: "%s"' % (self.robot_name, str(goal)))
        self.write_file()

    def write_file(self):
        """!
        Writes the values stored in the content dictionary, to a feedback file.
        Called every config.POLL_WRITER_SECONDS seconds.
        """
        if store.content._dictionary[self.robot_name]["position"].pos_x != None or \
            store.content._dictionary[self.robot_name]["battery"].percentage_charge != None or \
            store.content._dictionary[self.robot_name]["goal"].goal_status != -1    :
            filename = self.feedbackWriter.write_file(self.robot_name,
                    store.content._dictionary[self.robot_name]["position"],
                    store.content._dictionary[self.robot_name]["battery"],
                    store.content._dictionary[self.robot_name]["goal"],
                    "-best_effort")
            self.get_logger().debug(
                    "filename: %s "
                    "store._position: %s "
                    "store._battery: %s "
                    "store._goal: %s "
                    % (
                        filename,
                        store.content._dictionary[self.robot_name]["position"],
                        store.content._dictionary[self.robot_name]["battery"],
                        store.content._dictionary[self.robot_name]["goal"]
                        ) 
                    )

def main(args=sys.argv):
    rclpy.init(args=args)
    if(len(args) > 4):
        robot_information_reader = RobotInformationReader(namespace=args[1],
                robot_name=args[2], origin_x=args[3], origin_y=args[4],
                rotation=args[5])
    else:
        robot_information_reader = RobotInformationReader(namespace=args[1],
                robot_name=args[2])
    rclpy.spin(robot_information_reader)
    robot_information_reader.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
