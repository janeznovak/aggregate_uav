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
    echo "Cleaned Storage folder"
		ros2 launch create3_poc_didattico create3_board.launch.py namespace:=$1 x:=$2 y:=$3 yaw:=$4
		else
			echo "Required dependencies are missing"
fi
