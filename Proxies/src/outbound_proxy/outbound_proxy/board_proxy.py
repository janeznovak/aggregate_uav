import sys
import os
import rclpy
from rclpy.node import Node
from storage_msgs.msg import File
from rclpy.qos import *
from . import config
from . import proxy
from pathlib import Path
from rclpy.qos import qos_profile_sensor_data

class BoardOutboundProxy(proxy.OutboundProxy):
    """!
    Class extending ROS 2 Node responsible for publishing storage_msgs.msg.File
    messages created based on files.
    """
    def initialize_timers_and_publisher(self):
        """!
        Override
        Initialize timers and file publisher based on the selected mode.
        """
        self.timer_reader = self.create_timer(config.POLL_WRITER_SECONDS, self.publish_feedback_reader)
        self.timer_ap = self.create_timer(config.POLL_WRITER_SECONDS, self.publish_action_ap)
        self.best_effort_file_publisher = self.create_publisher(File,
                config.FROM_ROBOT_TOPIC_BEST_EFFORT, qos_profile_sensor_data) #best effort
        self.reliable_file_publisher = self.create_publisher(File,
                config.FROM_ROBOT_TOPIC_RELIABLE, config.DEFAULT_QOS_TOPIC) #reliable

    def publish_feedback_reader(self):
        """!
        Publish File messages based on feedback files coming from RobotReader.
        """
        self.publish_file(config.ROBOT_FEEDBACK_INPUT_PATH_FROM_READER)

    def publish_action_ap(self):
        """!
        Publish File messages based on action files coming from AP.
        """
        self.publish_file(config.ROBOT_ACTION_INPUT_PATH_FROM_AP)


    def publish_file(self, input_path):
        """!
        Override
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
                    if file_name.endswith('-reliable.txt'):
                        self.reliable_file_publisher.publish(msg)
                    if file_name.endswith('-best_effort.txt'):
                        self.best_effort_file_publisher.publish(msg)
                Path(file_path).unlink(missing_ok=True)
                return


def main(args=sys.argv):
    domain_id = 1
    if len(args) > 3:
        domain_id = int(args[2])
    os.environ['ROS_DOMAIN_ID'] = str(domain_id)

    rclpy.init(args=args)
    outproxy = BoardOutboundProxy(robot_name=args[1])
    rclpy.spin(outproxy)
    outproxy.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
