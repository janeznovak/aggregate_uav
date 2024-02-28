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
    DeclareLaunchArgument('robot_name', default_value='',
                          description='Robot name'),
    DeclareLaunchArgument('ros_domain', default_value='100',
                          description='ROS_DOMAIN_ID for proxies'),
    DeclareLaunchArgument('backup_storage', default_value='true',
                          choices=['true', 'false'],
                          description='Whether to save past action file.'),
    DeclareLaunchArgument('use_proxies', default_value='true',
                          choices=['true', 'false'],
                          description='Whether to enable ROS2 proxies.'),
    DeclareLaunchArgument('board_proxies', default_value='true',
                          choices=['true', 'false'],
                          description='Whether proxies are used in board or server mode.'),
]

for pose_element in ['x', 'y', 'z', 'yaw']:
    ARGUMENTS.append(DeclareLaunchArgument(pose_element, default_value='0.0',
                     description=f'{pose_element} component of the robot pose.'))


# Rviz requires US locale to correctly display the wheels
os.environ['LC_NUMERIC'] = 'en_US.UTF-8'


def generate_launch_description():
    # Launch configurations
    namespace = LaunchConfiguration('namespace')
    robot_name = LaunchConfiguration('robot_name')
    ros_domain = LaunchConfiguration('ros_domain')
    backup_storage = LaunchConfiguration('backup_storage')
    use_proxies = LaunchConfiguration('use_proxies')
    x, y, z = LaunchConfiguration('x'), LaunchConfiguration('y'), LaunchConfiguration('z')
    yaw = LaunchConfiguration('yaw')
    board_proxies = LaunchConfiguration('board_proxies')

    spawn_robot_group_action = GroupAction([
        PushRosNamespace(''),

        # requires source Robot_Reader
        Node(
            package="robot_reader",
            executable="robot_reader",
            output='screen',
            arguments=[namespace, robot_name, x, y, yaw]
        ),

        # requires source Robot_Writer
        Node(
            package="robot_writer",
            executable="robot_writer",
            output='screen',
            parameters=[
                {'backup_storage': backup_storage}
                ],
            arguments=[namespace, robot_name, x, y, yaw]
        ),

        Node(
            package="create3_poc_didattico",
            executable="main_node",
            output="screen",
            parameters=[
                {"x": x},
                {"y": y},
                {"namespace": ''} #set to '' and revert reset in main_node.cpp
                ],
        ),
    ])

    proxies = GroupAction([
        # Require source Proxies
        Node(
            package="inbound_proxy",
            executable="board_inbound_proxy",
            output='screen',
            arguments=[robot_name, ros_domain]
            ),

        # Require source Proxies
        Node(
            package="outbound_proxy",
            executable="board_outbound_proxy",
            output='screen',
            arguments=[robot_name, ros_domain]
            )
        ],
        condition=IfCondition(use_proxies)
        )


    # Define LaunchDescription variable
    ld = LaunchDescription(ARGUMENTS)
    ld.add_action(spawn_robot_group_action)
    ld.add_action(proxies)
    return ld
