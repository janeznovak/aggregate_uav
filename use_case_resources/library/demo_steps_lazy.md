# Steps for library demo

1. ITEM\_7 -> robot5 moves towards the goal
2. ITEM\_1 -> robot1 moves towards the goal
3. ITEM\_2 -> robot2 moves towards the goal
1. ITEM\_5 -> robot5 starts to move towards the goal
1. After robot5's departure, we invoke manually the script to simulate robot5 failure
	- ```./PoC/out_of_order.bash tb3_5 ```
	- Another robot will be selected for the task previously assigned to robot5
2. we can wait for robot2's battery to go under 90% charge level in order to favor the selection of robot4 for the next task
	1. Or we can discharge it manually
	2. ```
	   source GAZEBO_Turtlebot3/install/local_setup.bash
	   ros2 service call /tb3_2/gazebo_ros_battery/set_charge battery_services/srv/SetCharge "{charge: {data: -1.0}}"
		```
1. ITEM\_8 -> will be assigned to robot4 since it has a higher battery charge level
1. DOCK\_3 -> in order to move robot3 under the table
1. DOCK\_5 -> robot 3 will navigate between the chairs
