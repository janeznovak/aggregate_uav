#!/usr/bin/env python3
# Copyright 2021 iRobot Corporation. All Rights Reserved.
# @author Luis Enrique Chico Capistrano (lchico@irobot.com)

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import EnvironmentVariable, PathJoinSubstitution


bringup_dir = get_package_share_directory('turtlebot3_gazebo')
ARGUMENTS = []

# Set the robot and dock pose close to the wall by default
for pose_element, default_value in zip(['x', 'y', 'yaw'], ['9.0', '0.0', '0.0']):
    ARGUMENTS.append(DeclareLaunchArgument(pose_element, default_value=default_value,
                     description=f'{pose_element} component of the robot pose.'))


def generate_launch_description():
    # Directories
    pkg_create3_poc = get_package_share_directory('create3_poc_didattico')

    # Paths
    create3_launch_file = PathJoinSubstitution(
        [pkg_create3_poc, 'launch', 'create3_gazebo.launch.py'])
    world_path = PathJoinSubstitution([bringup_dir, 'worlds', 'biblioteca.sdf'])
    aws_model_path = PathJoinSubstitution([bringup_dir, 'models:'])

    # Includes
    world_spawn = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([create3_launch_file]),
        launch_arguments={'world_path': world_path}.items())

    # Add AWS models to gazebo path
    # This environment variable needs to be set, otherwise code fails
    set_gazebo_model_path_env = SetEnvironmentVariable(
        name='GAZEBO_MODEL_PATH',
        value=[EnvironmentVariable('GAZEBO_MODEL_PATH', default_value=''), aws_model_path])

    # Define LaunchDescription variable
    ld = LaunchDescription(ARGUMENTS)
    # Add actions to LaunchDescription
    ld.add_action(set_gazebo_model_path_env)
    ld.add_action(world_spawn)

    return ld
