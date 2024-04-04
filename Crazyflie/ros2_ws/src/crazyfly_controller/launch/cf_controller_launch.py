from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
    Node(
        package='crazyfly_controller',
        executable='crazyfly_controller',
    ),
    ])