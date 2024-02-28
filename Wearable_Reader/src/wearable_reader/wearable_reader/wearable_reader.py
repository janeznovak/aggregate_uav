# Copyright © 2023 University of Turin, Daniele Bortoluzzi & Giordano Scarso. All Rights Reserved.

import sys
import rclpy
from rclpy.node import Node
from rclpy.executors import MultiThreadedExecutor

from std_msgs.msg import String
from nav_msgs.msg import Odometry
from wearable_interfaces.msg import Sensor, Trigger

from . import config
from .feedback_writer import write_file_feedback, write_file_sos
from robot_reader import coords

class WearableInformationReader(Node):
    """!
    Class extending WearableInformationReader to receive position data from the /odom topic
    """
    def __init__(self, namespace, wearable_name, origin_x=0.0, origin_y=0.0, rotation=0.0):
        """!
        Class' constructor requiring a namespace and optionally the wearable
        coordinates relative to the origin.
        """
        super().__init__('wearable_information_reader', namespace=namespace)
        self.wearable_name = wearable_name
        self.origin_x = float(origin_x)
        self.origin_y = float(origin_y)
        self.rotation = float(rotation)
        self.store = {'position': {}, 'sensors': {}, 'triggers': {}}
        self.timer = self.create_timer(config.POLL_WRITER_SECONDS, self.write_file_feedback)
        self.get_logger().info('Wearable reader initialized')
        self.subscription_position = self.position_subscribe(namespace)
        self.subscription_trigger = self.trigger_subscribe(namespace)
        self.subscription_sensors = self.sensors_subscribe(namespace)

    def write_file_feedback(self):
        """!
        Write in the Storage the wearable feedback file.
        """
        try:
            self.get_logger().debug('Written feedback file "%s"' % (
                write_file_feedback(self.wearable_name, self.store)
                ))
        except Exception as e:
            self.get_logger().warn(
                    'Exception occured while writing feedback file\n%s\n%s' % (
                        type(e), str(e)))

    def write_file_sos(self):
        """!
        Write in the Storage the SOS file.
        """
        try:
            self.get_logger().debug('Written sos file "%s"' % (
                write_file_sos(self.wearable_name, self.store)
                ))
        except Exception as e:
            self.get_logger().warn(
                    'Exception occured while writing sos file\n%s\n%s' % (
                        type(e), str(e)))

    def position_subscribe(self, namespace):
        """!
        Given a message with "Odometry", trigger a callback.
        """
        self.get_logger().info('Subscribing to: "%s/%s"' % (namespace, config.ODOM_TOPIC))
        return self.create_subscription(
                Odometry,
                config.ODOM_TOPIC,
                self.listener_position_callback,
                config.DEFAULT_QOS_TOPIC
                )

    def sensors_subscribe(self, namespace):
        """!
        Given a message with "Sensor", trigger a callback.
        """
        self.get_logger().info('Subscribing to: "%s/%s"' % (namespace, config.SENSOR_TOPIC))
        return self.create_subscription(
                Sensor,
                config.SENSOR_TOPIC,
                self.listener_sensors_callback,
                config.DEFAULT_QOS_TOPIC
                )

    def trigger_subscribe(self, namespace):
        """!
        Given a message with "Trigger", trigger a callback.
        """
        self.get_logger().info('Subscribing to: "%s/%s"' % (namespace, config.TRIGGER_TOPIC))
        return self.create_subscription(
                Trigger,
                config.TRIGGER_TOPIC,
                self.listener_triggers_callback,
                config.DEFAULT_QOS_TOPIC
                )

    def listener_position_callback(self, msg):
        """!
        Given a message with pose.pose.position, converts the coordinate
        changing the origin point from the wearable origin point to the global
        origin point. Callback for the position topic.
        """
        x, y, yaw = coords.rel2abs(msg.pose.pose.position.x, msg.pose.pose.position.y,
                msg.pose.pose.orientation.z, msg.pose.pose.orientation.w,
                self.origin_x, self.origin_y, self.rotation)
        self.store["position"] = {
                'wearable_name': self.wearable_name,
                'x': x, 'y': y, 'yaw': yaw
                }
        self.get_logger().debug('I heard position from %s: "%s"' % (self.wearable_name, str(self.store['position'])))

    def listener_sensors_callback(self, msg):
        """!
        Given a Feedback message store its content into self.store.
        Callback for the config.SENSOR_TOPIC topic.
        """
        self.store["sensors"] = {
                'wearable_code': msg.wearable_code.replace("/",""),
                'heart_rate_per_minute': msg.heart_rate_per_minute,
                'body_temperature': msg.body_temperature,
                'oxygenation': msg.oxygenation,
                'goal_status': msg.goal_status,
                'goal_code': msg.goal_code,
                'unix_timestamp': msg.unix_timestamp
                }
        self.get_logger().debug('I heard sensors from %s: "%s"' % (self.wearable_name, str(self.store['sensors'])))

    def listener_triggers_callback(self, msg):
        """!
        Given a Trigger message write the SOS file.
        Callback for the config.TRIGGER_TOPIC topic.
        """
        self.store["triggers"] = {
                'wearable_code': msg.wearable_code.replace("/",""),
                'priority': msg.priority,
                'heart_rate_per_minute': msg.heart_rate_per_minute,
                'body_temperature': msg.body_temperature,
                'oxygenation': msg.oxygenation,
                'accidental_fall_sos_triggered': msg.accidental_fall_sos_triggered,
                'heart_rate_sos_triggered': msg.heart_rate_sos_triggered,
                'body_temperature_sos_triggered': msg.body_temperature_sos_triggered,
                'oxygenation_sos_triggered': msg.oxygenation_sos_triggered,
                'follow_triggered': msg.follow_triggered,
                'unix_timestamp': msg.unix_timestamp
                }
        self.get_logger().debug('I heard triggers from %s: "%s"' % (self.wearable_name, str(self.store['triggers'])))
        self.write_file_sos()

def main(args=sys.argv):
    rclpy.init(args=args)
    if(len(args) > 4):
        wearable_information_reader = WearableInformationReader(namespace=args[1],
                wearable_name=args[2], origin_x=args[3], origin_y=args[4],
                rotation=args[5])
    else:
        wearable_information_reader = WearableInformationReader(namespace=args[1],
                wearable_name=args[2])
    rclpy.spin(wearable_information_reader)
    wearable_information_reader.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
