#!/bin/bash
if [ -f $(dirname "$0")/Robot_Reader/install/local_setup.bash ]; then
	. $(dirname "$0")/Robot_Reader/install/local_setup.bash
else
	echo "You need to compile Robot_Reader"
fi

if [ -f $(dirname "$0")/Wearable_Reader/install/local_setup.bash ]; then
	. $(dirname "$0")/Wearable_Reader/install/local_setup.bash
else
	echo "You need to compile Wearable_Reader"
fi

if [ -f $(dirname "$0")/Gazebo_Custom_Plugins/wearable_interfaces/install/local_setup.bash ]; then
	. $(dirname "$0")/Gazebo_Custom_Plugins/wearable_interfaces/install/local_setup.bash
else
	echo "You need to compile Gazebo_Custom_Plugins/wearable_interfaces"
fi

if [[ -f $(dirname "$0")/Robot_Reader/install/local_setup.bash ]] &&
  [[ -f $(dirname "$0")/Wearable_Reader/install/local_setup.bash ]] &&
  [[ -f $(dirname "$0")/Gazebo_Custom_Plugins/wearable_interfaces/install/local_setup.bash ]]; then
    		echo "Cleaning Storage folder"
        find $(dirname "$0")/../Storage/from_ap -type f -name '*.txt' -o -name '*.lock' -exec rm {} \;
        find $(dirname "$0")/../Storage/from_robot -type f -name '*.txt' -o -name '*.lock' -exec rm {} \;
        find $(dirname "$0")/../Storage/from_user -type f -name '*.txt' -o -name '*.lock' -exec rm {} \;
        find $(dirname "$0")/../Storage/from_wearable -type f -name '*.txt' -o -name '*.lock' -exec rm {} \;
        ros2 run wearable_reader wearable_reader $1 $1
		else
			echo "Required dependencies are missing"
fi
