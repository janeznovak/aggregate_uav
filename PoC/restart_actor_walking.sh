#!/bin/bash
robot_code=$1
wearable_code=$2
echo "Request to restart actor ($wearable_code) to walk again"

echo "Sleep for 20 seconds"
sleep 20

echo "Publish to $wearable_code"
source ../Gazebo_Custom_Plugins/wearable_interfaces/install/local_setup.bash
ros2 topic pub -1 /$wearable_code/patient_action std_msgs/msg/Int8 "data: 6"
