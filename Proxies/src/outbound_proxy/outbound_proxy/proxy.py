import sys
import os
import rclpy
from rclpy.node import Node
from storage_msgs.msg import File
from rclpy.qos import *
from . import config
from pathlib import Path
from rclpy.qos import qos_profile_sensor_data

class OutboundProxy(Node):
    """!
    Class extending ROS 2 Node responsible for publishing storage_msgs.msg.File
    messages created based on files.
    """

    def __init__(self, robot_name):
        """!
        Class constructor, requires a namespace and board mode flag.
        """
        super().__init__('outbound_proxy')
        self.robot_name = robot_name
        self.initialize_timers_and_publisher()
        self.get_logger().info('Outbound proxy initialized')

    def initialize_timers_and_publisher(self):
        """!
        Initialize timers and file publisher based on the selected mode.
        """
        self.get_logger().info('server mode')
        self.timer_server = self.create_timer(config.POLL_WRITER_SECONDS,
                self.publish_goal)
        self.file_publisher = self.create_publisher(File,
                config.TO_ROBOT_TOPIC, config.DEFAULT_QOS_TOPIC) #reliable

    def publish_goal(self):
        """!
        Publish File messages based on action files coming from server.
        """
        self.publish_file(config.GOAL_INPUT_PATH_FROM_SERVER)

    def publish_file(self, input_path):
        """!
        Given an input path produces and publishes at most one File message.
        """
        target = input_path.replace(config.ROBOT_PLACEHOLDER, self.robot_name)
        path_tmp = os.path.join(config.STORAGE_BASE_PATH, target)
        path_feedbacks = Path(__file__).parent / path_tmp
        path_feedbacks.mkdir(exist_ok=True, parents=True)
        path = path_feedbacks.absolute()

        for file_name in os.listdir(path):
            if file_name.endswith('txt'):
                file_path = os.path.join(path, file_name)
                with open(file_path, 'r') as feedback_file:
                    msg = File()
                    msg.path = os.path.join(path_tmp, file_name).replace("../", "")
                    msg.charset = "UTF8"
                    msg.data = [bytes(x, 'utf-8') for x in feedback_file.read()]
                    self.file_publisher.publish(msg)
                Path(file_path).unlink(missing_ok=True)
                return

def main(args=sys.argv):
    domain_id = 1
    if len(args) > 3:
        domain_id = int(args[2])
    os.environ['ROS_DOMAIN_ID'] = str(domain_id)

    rclpy.init(args=args)
    outproxy = OutboundProxy(robot_name=args[1])
    rclpy.spin(outproxy)
    outproxy.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
