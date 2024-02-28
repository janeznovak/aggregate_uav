"""!
Configuration constant used by inbound_proxy in board config.
"""
import rclpy.qos as qos

DEFAULT_QOS_TOPIC       = qos.QoSProfile(depth=10, durability=1)
POLL_WRITER_SECONDS     = 0.5
ROOT_BASE_PATH          = "../../../../"
TARGET_TOPIC            = "/proxy/message_to_robot"
