#!/bin/bash
robot_code=$1
# ros2 service call /$robot_code/gazebo_ros_battery/set_charge battery_services/srv/SetCharge "{charge: {data: -3.0}}"
ros2 topic pub /robot_code/battery_state sensor_msgs/msg/BatteryState "{charge: -3.0, capacity: 100.0, power_supply_status: 100}"
