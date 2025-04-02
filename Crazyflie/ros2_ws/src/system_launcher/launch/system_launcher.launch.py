import os
import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, TimerAction
from launch_ros.actions import Node
from launch.conditions import LaunchConfigurationEquals


def generate_launch_description():
    # load crazyflies
    crazyflies_yaml = os.path.join(
        get_package_share_directory("system_launcher"), "config", "crazyfliesConfig_lab.yaml"
    )

    with open(crazyflies_yaml, "r") as ymlfile:
        crazyflies = yaml.safe_load(ymlfile)

    # server params
    server_yaml = os.path.join(
        get_package_share_directory("system_launcher"), "config", "server.yaml"
    )

    with open(server_yaml, "r") as ymlfile:
        server_yaml_contents = yaml.safe_load(ymlfile)

    server_params = [crazyflies] + [
        server_yaml_contents["/crazyflie_server"]["ros__parameters"]
    ]

    # robot description
    urdf = os.path.join(
        get_package_share_directory("crazyflie"), "urdf", "crazyflie_description.urdf"
    )
    with open(urdf, "r") as f:
        robot_desc = f.read()
    server_params[1]["robot_description"] = robot_desc

    server_node = [
        DeclareLaunchArgument(name='log_level', default_value='debug'),
        DeclareLaunchArgument("backend", default_value="cflib"),
        Node(
            package="crazyflie",
            executable="crazyflie_server.py",
            condition=LaunchConfigurationEquals("backend", "cflib"),
            name="crazyflie_server",
            output="screen",
            parameters=server_params
        ),
        Node(
            package="rviz2",
            namespace="",
            executable="rviz2",
            name="rviz2",
            arguments=[
                "-d"
                + os.path.join(
                    get_package_share_directory("crazyflie"), "config", "config.rviz"
                )
            ],
            parameters=[
                {
                    "use_sim_time": True,
                }
            ],
        ),
    ]

    robots_node = []
    time = 0.0
    for robot_name, robot_info in crazyflies['robots'].items():
        if str(robot_info['enabled']) == "True":
            group = GroupAction([
                Node(
                    package="crazyflie_controller",
                    executable="run_many",
                    name="controller_node",
                    output="screen",
                    arguments=[robot_name],
                ),
                Node(
                    package="robot_reader",
                    executable="robot_reader",
                    output="screen",
                    arguments=[robot_name, robot_name],
                ),
                Node(
                    package="robot_writer",
                    executable="robot_writer",
                    output='screen',
                    arguments=[robot_name, robot_name]
                )
            ])
            timed_group = TimerAction(
                period=time,
                actions=[group]
            )
            time += 3.0
            robots_node.append(timed_group)

    delay_action_robots = TimerAction(
            period=7.0,  
            actions=robots_node
    )

    return LaunchDescription(server_node + [delay_action_robots])
