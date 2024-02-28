"""!
Configuration constant used by inbound_proxy in server config.
"""
import rclpy.qos as qos

DEFAULT_BEST_EFFORT_QOS_TOPIC   = qos.qos_profile_sensor_data
DEFAULT_RELIABLE_QOS_TOPIC      = qos.QoSProfile(depth=10, durability=1)
POLL_WRITER_SECONDS     = 0.5
ROOT_BASE_PATH          = "../../../../"
TARGET_TOPIC_BEST_EFFORT= "/proxy/message_from_robot_best_effort"
TARGET_TOPIC_RELIABLE   = "/proxy/message_from_robot_reliable"
