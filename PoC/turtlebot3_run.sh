#!/bin/bash
if [ -f $(dirname "$0")/Navigation_System/install/local_setup.bash ]; then
	. $(dirname "$0")/Navigation_System/install/local_setup.bash
else
	echo "You need to compile Navigation_System"
fi

if [ -f $(dirname "$0")/Robot_Reader/install/local_setup.bash ]; then
	. $(dirname "$0")/Robot_Reader/install/local_setup.bash
else
	echo "You need to compile Robot_Reader"
fi

if [ -f $(dirname "$0")/Robot_Writer/install/local_setup.bash ]; then
	. $(dirname "$0")/Robot_Writer/install/local_setup.bash
else
	echo "You need to compile Robot_Writer"
fi

if [ -f $(dirname "$0")/Proxies/install/local_setup.bash ]; then
	. $(dirname "$0")/Proxies/install/local_setup.bash
else
	echo "You need to compile Proxies"
fi

if [ -f $(dirname "$0")/GAZEBO_Turtlebot3/install/local_setup.bash ]; then
	. $(dirname "$0")/GAZEBO_Turtlebot3/install/local_setup.bash
else
	echo "You need to compile GAZEBO_Turtlebot3"
fi

if [[ -f $(dirname "$0")/Navigation_System/install/local_setup.bash ]] &&
		[[ -f $(dirname "$0")/Robot_Reader/install/local_setup.bash ]] &&
		[[ -f $(dirname "$0")/Robot_Writer/install/local_setup.bash ]] &&
		[[ -f $(dirname "$0")/Proxies/install/local_setup.bash ]] &&
		[[ -f $(dirname "$0")/GAZEBO_Turtlebot3/install/local_setup.bash ]]; then
        echo "Cleaning Storage folder"
        find "$(dirname "$0")/../Storage/from_ap" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_ap" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
        find "$(dirname "$0")/../Storage/from_robot" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_robot" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
        find "$(dirname "$0")/../Storage/from_user" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_user" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
        find "$(dirname "$0")/../Storage/from_wearable" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_wearable" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
        echo "Cleaned Storage folder"
        TURTLEBOT3_MODEL=burger ROBOTS_YAML=use_case_resources/library/robots.yaml \
          ros2 launch turtlebot3_gazebo turtlebot3_library.launch.py use_proxies:=false decrease_battery:=True
		else
			echo "Required dependencies are missing"
fi
