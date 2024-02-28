import sys
import os
import rclpy
from rclpy.node import Node
from storage_msgs.msg import File
from rclpy.qos import *

import time, threading, string

from pathlib import Path
from . import board_config as config
from . import proxy

class BoardInboundProxy(proxy.InboundProxy):
    """!
    Class extending ROS2 Node use to convert message received
    FROM topic INTO file.
    """

    def create_subscribers(self):
        self.get_logger().info('Target topic: ' + config.TARGET_TOPIC)
        self.file_message_subscription = self.create_subscription(
            File, config.TARGET_TOPIC, self.message_received,
            config.DEFAULT_QOS_TOPIC)


def main(args=sys.argv):
    domain_id = 1
    if len(args) > 3:
        #TODO regex per controllare ROS_DOMAIN_ID
        domain_id = int(args[2])
    os.environ['ROS_DOMAIN_ID'] = str(domain_id)

    rclpy.init(args=args)
    inproxy = BoardInboundProxy(robot_name=args[1])
    rclpy.spin(inproxy)
    inproxy.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
