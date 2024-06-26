"""!
Module with configuration constant used in the robot_reader package.
"""

# Copyright © 2023 University of Turin, Daniele Bortoluzzi & Giordano Scarso. All Rights Reserved.

from rclpy.qos import qos_profile_services_default

ODOM_TOPIC = "odom"
POSE_TOPIC = "pose"
BATTERY_TOPIC = "battery_state"
GOAL_TOPIC = "goal_state"

DEFAULT_QOS_TOPIC = qos_profile_services_default
BATTERY_QOS_TOPIC = qos_profile_services_default

POLL_WRITER_SECONDS = 0.1

ROBOTS_PREFIX = "cf_"
ROBOTS_LENGTH = 10
ROBOTS = [
    dict(name=ROBOTS_PREFIX + str(i), base=dict(x=0, y=0, w=0))
    for i in range(0, ROBOTS_LENGTH)
]

# EXAMPLE:
# [
#     {
#         "name": "tb3_1",
#         "base": {
#             "x": 0,
#             "y": 0,
#             "w": 0
#         }
#     },
#     {
#         "name": "tb3_2",
#         "base": {
#             "x": 0,
#             "y": 0,
#             "w": 0
#         }
#     },
#     {
#         "name": "tb3_3",
#         "base": {
#             "x": 0,
#             "y": 0,
#             "w": 0
#         }
#     }
# ]
