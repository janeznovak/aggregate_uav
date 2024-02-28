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

if [ -f $(dirname "$0")/GAZEBO_Create3/install/local_setup.bash ]; then
	. $(dirname "$0")/GAZEBO_Create3/install/local_setup.bash
else
	echo "You need to compile GAZEBO_Create3"
fi

if [[ -f $(dirname "$0")/Navigation_System/install/local_setup.bash ]] &&
		[[ -f $(dirname "$0")/Robot_Reader/install/local_setup.bash ]] &&
		[[ -f $(dirname "$0")/Robot_Writer/install/local_setup.bash ]] &&
		[[ -f $(dirname "$0")/Proxies/install/local_setup.bash ]] &&
		[[ -f $(dirname "$0")/GAZEBO_Create3/install/local_setup.bash ]]; then
      echo "Cleaning Storage folder"
      find "$(dirname "$0")/../Storage/from_ap" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_ap" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
      find "$(dirname "$0")/../Storage/from_robot" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_robot" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
      find "$(dirname "$0")/../Storage/from_user" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_user" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
      find "$(dirname "$0")/../Storage/from_wearable" -depth -type f \( -name '*.txt' -o -name '*.lock' \) -delete -o -path "$(dirname "$0")/Storage/from_wearable" -prune -type f \( -name '*.txt' -o -name '*.lock' \) -delete
      echo "Cleaned Storage folder"
      ros2 launch create3_poc_didattico create3_gazebo.launch.py namespace:=$1 robot_name:=$2 use_rviz:=false visualize_rays:=false x:=$3 y:=$4 yaw:=$5 $(if [ "$6" ]; then echo "use_proxies:=$6"; fi)
		else
			echo "Required dependencies are missing"
fi
