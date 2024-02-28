#!/bin/bash
wearable_code=$1
trigger_event=$2
priority=$3 # at the moment it's not used

trigger_desc=""

case $trigger_event in
    1)
        trigger_desc="Accidental_fall_sos_triggered"
        ;;
    2)
        trigger_desc="Oxygenation_sos_triggered"
        ;;
    3)
        trigger_desc="Body_temperature_sos_triggered"
        ;;
    4)
        trigger_desc="Heart_rate_sos_triggered"
        ;;
    5)
        trigger_desc="Manual calling"
        ;;
    *)
        trigger_desc="Invalid code"
        ;;
esac

echo "Request to trigger to actor ($wearable_code) an event with code $trigger_event and description '$trigger_desc'"

echo "Publish to $wearable_code"
source ../Gazebo_Custom_Plugins/wearable_interfaces/install/local_setup.bash
ros2 topic pub -1 /$wearable_code/patient_action std_msgs/msg/Int8 "data: $trigger_event"
