from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction
from launch_ros.actions import Node, PushRosNamespace
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    target_x = LaunchConfiguration('x')
    target_y = LaunchConfiguration('y')
    namespace = LaunchConfiguration('namespace')
    target_x_arg = DeclareLaunchArgument(
            'x',
            default_value='0.0'
            )
    target_y_arg = DeclareLaunchArgument(
            'y',
            default_value='0.0'
            )
    namespace_arg = DeclareLaunchArgument(
            'namespace', default_value='')
    ld = LaunchDescription()
    ld.add_action(target_x_arg)
    ld.add_action(target_y_arg)
    ld.add_action(namespace_arg)
    ld.add_action(
            GroupAction([
                PushRosNamespace(namespace=namespace),
                LaunchDescription([
                    Node(
                        package="create3_poc_didattico",
                        executable="main_node",
                        output="screen",
                        parameters=[
                            {"x": target_x},
                            {"y": target_y},
                            {"namespace": namespace}
                            ],
                        )
                    ])
                ])
            )
    return ld
