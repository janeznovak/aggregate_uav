import sys
import os
import rclpy
from rclpy.node import Node
from storage_msgs.msg import File
from rclpy.qos import *

import time, threading, string

from pathlib import Path
from . import server_config as config

class InboundProxy(Node):
    """!
    Class extending ROS2 Node use to convert message received
    FROM topic INTO file.
    """

    def __init__(self, robot_name):
        super().__init__('inbound_proxy')
        self.robot_name = robot_name
        self.create_subscribers()
        self.get_logger().info('Inbound proxy initialized')

    def create_subscribers(self):
        self.get_logger().info('Target topic: ' + config.TARGET_TOPIC_BEST_EFFORT)
        self.feedback_file_message_subscription = self.create_subscription(
            File, config.TARGET_TOPIC_BEST_EFFORT, self.message_received,
            config.DEFAULT_BEST_EFFORT_QOS_TOPIC)
        self.get_logger().info('Target topic: ' + config.TARGET_TOPIC_RELIABLE)
        self.action_file_message_subscription = self.create_subscription(
            File, config.TARGET_TOPIC_RELIABLE, self.message_received,
            config.DEFAULT_RELIABLE_QOS_TOPIC)


    def message_received(self, msg: File):
        """!
        Given a storage_msgs.msg.File message writes its content into the specified path
        as a file.
        """
        msg_path = os.path.split(msg.path)
        path_tmp = os.path.join(config.ROOT_BASE_PATH, msg_path[0])
        target_path = Path(__file__).parent / path_tmp
        target_path.parent.mkdir(exist_ok=True, parents=True)
        path = target_path.absolute()
        text = b''.join(msg.data)
        file_path = Path(os.path.join(path, msg_path[1]))
        with open(file_path.with_suffix('.lock'), 'wb') as file_to_write:
            self.get_logger().info('Received new file from ' + file_path.as_posix())
            file_to_write.write(text)
            try:
                os.rename(file_path.with_suffix('.lock'), file_path)
            except:
                print("Warning: rename to " + file_path.as_posix() + " failed")

def main(args=sys.argv):
    domain_id = 1
    if len(args) > 3:
        #TODO regex per controllare ROS_DOMAIN_ID
        domain_id = int(args[2])
    os.environ['ROS_DOMAIN_ID'] = str(domain_id)

    rclpy.init(args=args)
    inproxy = InboundProxy(robot_name=args[1])
    rclpy.spin(inproxy)
    inproxy.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
