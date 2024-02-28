#!/usr/bin/env python3
# Copyright 2021 iRobot Corporation. All Rights Reserved.
# @author Rodrigo Jose Causarano Nunez (rcausaran@irobot.com)
#
# Launch Create(R) 3 in Gazebo and optionally also in RViz.

import os

from ament_index_python.packages import get_package_share_directory

from irobot_create_common_bringup.namespace import GetNamespacedName
from irobot_create_common_bringup.offset import OffsetParser, RotationalOffsetX, RotationalOffsetY

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction
from launch.actions import IncludeLaunchDescription
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node, PushRosNamespace


ARGUMENTS = [
    DeclareLaunchArgument('namespace', default_value='',
                          description='Robot namespace'),
    DeclareLaunchArgument('ros_domain', default_value='100',
                          description='ROS_DOMAIN_ID for proxies'),
    DeclareLaunchArgument('use_proxies', default_value='true',
                          choices=['true', 'false'],
                          description='Whether to enable ROS2 proxies.'),
    DeclareLaunchArgument('board_proxies', default_value='false',
                          choices=['true', 'false'],
                          description='Whether proxies are used in board or server mode.'),
]



# Rviz requires US locale to correctly display the wheels
os.environ['LC_NUMERIC'] = 'en_US.UTF-8'


def generate_launch_description():
    # Launch configurations
    namespace = LaunchConfiguration('namespace')
    ros_domain = LaunchConfiguration('ros_domain')
    use_proxies = LaunchConfiguration('use_proxies')
    board_proxies = LaunchConfiguration('board_proxies')

    proxies = GroupAction([
        # Require source Proxies
        Node(
            name="inbound_proxy_server",
            package="inbound_proxy",
            executable="server_inbound_proxy",
            output='screen',
            arguments=[namespace, ros_domain]
            ),

        # Require source Proxies
        Node(
            name="outbound_proxy_server",
            package="outbound_proxy",
            executable="server_outbound_proxy",
            output='screen',
            arguments=[namespace, ros_domain]
            )
        ],
        condition=IfCondition(use_proxies)
        )


    # Define LaunchDescription variable
    ld = LaunchDescription(ARGUMENTS)
    ld.add_action(proxies)
    return ld
