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

if [ -f $(dirname "$0")/Crazyflie/ros2_ws/install/local_setup.bash ]; then
	. $(dirname "$0")/Crazyflie/ros2_ws/install/local_setup.bash 
else
	echo "You need to compile Crazyflie"
fi


if [[ -f $(dirname "$0")/Navigation_System/install/local_setup.bash ]] &&
		[[ -f $(dirname "$0")/Robot_Reader/install/local_setup.bash ]] &&
		[[ -f $(dirname "$0")/Robot_Writer/install/local_setup.bash ]] &&
		[[ -f $(dirname "$0")/Crazyflie/ros2_ws/install/local_setup.bash ]]; then
        echo "Cleaning Storage folder"
        find "$(dirname "$0")/../Storage/from_ap" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_ap" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
        find "$(dirname "$0")/../Storage/from_robot" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_robot" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
        find "$(dirname "$0")/../Storage/from_user" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_user" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
        echo "Cleaned Storage folder"
        
		echo "Launching test.."
		ros2 launch system_launcher system_launcher.launch.py
		
		else
			echo "Required dependencies are missing"
fi
