#!/usr/bin/env python3
# Copyright 2021 iRobot Corporation. All Rights Reserved.
# @author Rodrigo Jose Causarano Nunez (rcausaran@irobot.com)
#
# Launch Create(R) 3 in Gazebo and optionally also in RViz.

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution


ARGUMENTS = [
    DeclareLaunchArgument('use_rviz', default_value='true',
                          choices=['true', 'false'],
                          description='Start rviz.'),
    DeclareLaunchArgument('use_gazebo_gui', default_value='true',
                          choices=['true', 'false'],
                          description='Set "false" to run gazebo headless.'),
    DeclareLaunchArgument('spawn_dock', default_value='true',
                          choices=['true', 'false'],
                          description='Spawn the standard dock model.'),
    DeclareLaunchArgument('world_path', default_value='',
                          description='Set world path, by default is empty.world'),
    DeclareLaunchArgument('namespace', default_value='',
                          description='Robot namespace'),
    DeclareLaunchArgument('ros_domain', default_value='100',
                          description='ROS_DOMAIN_ID for proxies'),
    DeclareLaunchArgument('backup_storage', default_value='true',
                          choices=['true', 'false'],
                          description='Whether to save past action file.'),
    DeclareLaunchArgument('use_proxies', default_value='true',
                          choices=['true', 'false'],
                          description='Whether to enable ROS2 proxies.'),
]

for pose_element in ['x', 'y', 'z', 'yaw']:
    ARGUMENTS.append(DeclareLaunchArgument(pose_element, default_value='0.0',
                     description=f'{pose_element} component of the robot pose.'))


def generate_launch_description():
    # Directories
    pkg_create3_gazebo_bringup = get_package_share_directory('irobot_create_gazebo_bringup')
    pkg_create3_poc = get_package_share_directory('create3_poc_didattico')

    # Paths
    gazebo_launch = PathJoinSubstitution(
        [pkg_create3_gazebo_bringup, 'launch', 'gazebo.launch.py'])
    robot_spawn_launch = PathJoinSubstitution(
        [pkg_create3_poc, 'launch', 'create3_physical_spawn.launch.py'])

    robot_spawn = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([robot_spawn_launch]),
        launch_arguments=[
            ('namespace', LaunchConfiguration('namespace')),
            ('use_rviz', LaunchConfiguration('use_rviz')),
            ('use_proxies', LaunchConfiguration('use_proxies')),
            ('ros_domain', LaunchConfiguration('ros_domain')),
            ('backup_storage', LaunchConfiguration('backup_storage')),
            ('x', LaunchConfiguration('x')),
            ('y', LaunchConfiguration('y')),
            ('z', LaunchConfiguration('z')),
            ('yaw', LaunchConfiguration('yaw'))])

    # Define LaunchDescription variable
    ld = LaunchDescription(ARGUMENTS)
    # Robot spawn
    ld.add_action(robot_spawn)
    return ld
