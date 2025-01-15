import os
import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, TimerAction
from launch_ros.actions import Node
from launch.conditions import LaunchConfigurationEquals


def generate_launch_description():
    pkg_system_launcher = get_package_share_directory("system_launcher")
    pgk_crazyflie = get_package_share_directory("crazyflie")
    sdf_robot_file = os.path.join(pkg_system_launcher, "models", "robot.sdf")
    
    # load Crazyflie configuaration
    crazyflies_yaml = os.path.join(
        pkg_system_launcher, "config", "crazyfliesConfig.yaml"
    )
    with open(crazyflies_yaml, "r") as ymlfile:
        crazyflies = yaml.safe_load(ymlfile)

    # load server configuration
    server_yaml = os.path.join(
        pkg_system_launcher, "config", "server.yaml"
    )
    with open(server_yaml, "r") as ymlfile:
        server_yaml_contents = yaml.safe_load(ymlfile)

    server_params = [crazyflies] + [
        server_yaml_contents["/crazyflie_server"]["ros__parameters"]
    ]

    # load Crazyflie robot description
    urdf = os.path.join(
        pgk_crazyflie, "urdf", "crazyflie_description.urdf"
    )
    with open(urdf, "r") as f:
        robot_desc = f.read()
    server_params[1]["robot_description"] = robot_desc
    
    num_custom_robot = 1
    
    nodes = []

    # Crazyflie server and rviz
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
                    pgk_crazyflie, "config", "config.rviz"
                )
            ],
            parameters=[
                {
                    "use_sim_time": True,
                }
            ],
        ),
    ]
    nodes.extend(server_node)

    # spawn Crazyflie robots
    robots_node = []
    time = 0.0
    for i in range(num_custom_robot):
        robot_namespace = f'worker_{i}'
        gz_topic = f'/model/{robot_namespace}'
        joint_state_gz_topic = f'/world/demo{gz_topic}/joint_state'
        link_pose_gz_topic = f'{gz_topic}/pose'

        # Modified parameter bridge for custom robot
        bridge = Node(
            package='ros_gz_bridge',
            executable='parameter_bridge',
            namespace=robot_namespace,
            arguments=[
                '/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock',
                f'{joint_state_gz_topic}@sensor_msgs/msg/JointState[gz.msgs.Model',
                f'{link_pose_gz_topic}@tf2_msgs/msg/TFMessage[gz.msgs.Pose_V',
                f'{gz_topic}/cmd_vel@geometry_msgs/msg/Twist@gz.msgs.Twist',
                f'{gz_topic}/odometry@nav_msgs/msg/Odometry@gz.msgs.Odometry',
                f'lidar@sensor_msgs/msg/LaserScan@gz.msgs.LaserScan',
                f'lidar/points@sensor_msgs/msg/PointCloud2@gz.msgs.PointCloudPacked'
            ],
            remappings=[
                (joint_state_gz_topic, 'joint_states'),
                (f'{gz_topic}/tf', 'tf'),
                (link_pose_gz_topic, 'pose'),
            ],
            parameters=[
                {'use_sim_time': True}
            ],
            output='screen'
        )

        # Read custom robot description
        with open(sdf_robot_file, 'r') as infp:
            custom_robot_desc = infp.read()

        # Modified robot state publisher for custom robot
        robot_state_publisher = Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            namespace=robot_namespace,
            name=f'robot_state_publisher_{i}',
            output='both',
            parameters=[
                {'use_sim_time': True},
                {'robot_description': custom_robot_desc},
                {'frame_prefix': f'{robot_namespace}/'},
                {'publish_frequency': 30.0}
            ],
            arguments=['--ros-args', '--log-level', 'debug']
        )
        
        # Add transforms from world to odom and odom to base_link
        transform_publishers = [
            # world -> odom
            Node(
                package='tf2_ros',
                executable='static_transform_publisher',
                namespace=robot_namespace,
                name=f'world_to_odom_tf_{i}',
                arguments=[
                    '--x', '0',
                    '--y', '0',
                    '--z', '0',
                    '--qx', '0',
                    '--qy', '0',
                    '--qz', '0',
                    '--qw', '1',
                    '--frame-id', 'world',
                    '--child-frame-id', f'{robot_namespace}/odom'
                ]
            ),
            # odom -> chassis (base_link)
            Node(
                package='tf2_ros',
                executable='static_transform_publisher',
                namespace=robot_namespace,
                name=f'odom_to_base_tf_{i}',
                arguments=[
                    '--x', '0',
                    '--y', '0',
                    '--z', '0',
                    '--qx', '0',
                    '--qy', '0',
                    '--qz', '0',
                    '--qw', '1',
                    '--frame-id', f'{robot_namespace}/odom',
                    '--child-frame-id', f'{robot_namespace}/chassis'
                ]
            ),
            # chassis -> lidar_link
            Node(
                package='tf2_ros',
                executable='static_transform_publisher',
                namespace=robot_namespace,
                name=f'base_to_lidar_tf_{i}',
                arguments=[
                    '--x', '0.8',
                    '--y', '0.0',
                    '--z', '0.51',
                    '--qx', '0',
                    '--qy', '0',
                    '--qz', '0',
                    '--qw', '1',
                    '--frame-id', f'{robot_namespace}/chassis',
                    '--child-frame-id', f'{robot_namespace}/lidar_link'
                ]
            )
        ]
        
        nodes.extend([bridge, robot_state_publisher] + transform_publishers)

    return LaunchDescription(nodes)
