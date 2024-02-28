#!/bin/bash
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
find "$(dirname "$0")/../Storage/from_ap" -type f -name '*.txt' -o -name '*.lock' -exec rm {} \;
find "$(dirname "$0")/../Storage/from_robot" -type f -name '*.txt' -o -name '*.lock' -exec rm {} \;
find "$(dirname "$0")/../Storage/from_user" -type f -name '*.txt' -o -name '*.lock' -exec rm {} \;
find "$(dirname "$0")/../Storage/from_wearable" -type f -name '*.txt' -o -name '*.lock' -exec rm {} \;
world_file="use_case_resources/store/world.sdf"

TURTLEBOT3_MODEL=burger ROBOTS_YAML=use_case_resources/store/robots.yaml \
ros2 launch turtlebot3_gazebo turtlebot3_library.launch.py use_proxies:=false \
world:="$world_file" map:=use_case_resources/store/store_map.yaml robots:=use_case_resources/store/robots.yaml
