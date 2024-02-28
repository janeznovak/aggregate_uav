#!/bin/bash
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

if  [[ -f $(dirname "$0")/GAZEBO_Create3/install/local_setup.bash ]]
    [[ -f $(dirname "$0")/Proxies/install/local_setup.bash ]]; then
      ros2 launch create3_poc_didattico server_proxies.launch.py
		else
			echo "Required dependencies are missing"
fi
