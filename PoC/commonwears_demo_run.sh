#!/bin/bash

if [ "$1" == "true" ]; then
    automatic_trigger=true
elif [ "$1" == "false" ]; then
    automatic_trigger=false
else
    automatic_trigger=false
fi

source_local_setup() {
    if [ -f "$1" ]; then
        . "$1"
    else
        echo "You need to compile $2"
        exit 1
    fi
}

source_local_setup "$(dirname "$0")/Navigation_System/install/local_setup.bash" "Navigation_System"
source_local_setup "$(dirname "$0")/Robot_Reader/install/local_setup.bash" "Robot_Reader"
source_local_setup "$(dirname "$0")/Robot_Writer/install/local_setup.bash" "Robot_Writer"
source_local_setup "$(dirname "$0")/Proxies/install/local_setup.bash" "Proxies"
source_local_setup "$(dirname "$0")/GAZEBO_Turtlebot3/install/local_setup.bash" "GAZEBO_Turtlebot3"
source_local_setup "$(dirname "$0")/Gazebo_Custom_Plugins/install/local_setup.bash" "Gazebo_Custom_Plugins"
source_local_setup "$(dirname "$0")/Gazebo_Custom_Plugins/wearable_interfaces/install/local_setup.bash" "Gazebo_Custom_Plugins/wearable_interfaces"

echo "Cleaning Storage folder"
find "$(dirname "$0")/../Storage/from_ap" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_ap" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
find "$(dirname "$0")/../Storage/from_robot" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_robot" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
find "$(dirname "$0")/../Storage/from_user" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_user" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
find "$(dirname "$0")/../Storage/from_wearable" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_wearable" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
echo "Cleaned Storage folder"

if [ "$automatic_trigger" == true ]; then
    world_file="use_case_resources/hospital/world_automatic_trigger.sdf"
else
    world_file="use_case_resources/hospital/world.sdf"
fi

TURTLEBOT3_MODEL=burger ROBOTS_YAML=use_case_resources/hospital/robots.yaml \
ros2 launch turtlebot3_gazebo turtlebot3_library.launch.py use_proxies:=false \
world:="$world_file" map:=use_case_resources/hospital/hospital_map.yaml robots:=use_case_resources/hospital/robots.yaml
