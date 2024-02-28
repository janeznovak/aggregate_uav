"""!
Configuration constants used by outbound_proxy.
"""
import rclpy.qos as qos

# durability = 1 == TRANSIENT_LOCAL
DEFAULT_QOS_TOPIC                       = qos.QoSProfile(depth=10, durability=1)
POLL_WRITER_SECONDS                     = 0.5
ROBOT_PLACEHOLDER                       = "#ROBOT"
ROBOT_FEEDBACK_INPUT_PATH_FROM_READER   = "from_robot/#ROBOT/to_web/feedback/"
ROBOT_ACTION_INPUT_PATH_FROM_AP         = "from_ap/to_web/actions/#ROBOT/"
GOAL_INPUT_PATH_FROM_SERVER             = "from_user/goals/"
STORAGE_BASE_PATH                       = "../../../../Storage/"
FROM_ROBOT_TOPIC_BEST_EFFORT            = "proxy/message_from_robot_best_effort"
FROM_ROBOT_TOPIC_RELIABLE               = "proxy/message_from_robot_reliable"
TO_ROBOT_TOPIC                          = "proxy/message_to_robot"
