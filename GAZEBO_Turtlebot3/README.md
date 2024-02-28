# Gazebo Turtlebot3 simulation

## First installation

### Install gazebo
https://classic.gazebosim.org/tutorials?tut=install_ubuntu

Note that, as explained in the tutorial, starting from Ubuntu 22.04 the classic version of gazebo is available from the ubuntu packages instead of `packages.osrfoundation.org`.

### Install ROS2
https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debians.html (remember to install also development tools!)   
Eventually fix some dependency problems: https://askubuntu.com/questions/1378869/dependency-issue

### Other ROS2 dependencies
```bash
sudo apt install -y ros-humble-irobot-create-msgs
```

### Build
```bash

# source ros2 (add in .bashrc or .profile)
source /opt/ros/humble/setup.bash

# export TURTLEBOT3_MODEL (add in .bashrc or .profile)
export TURTLEBOT3_MODEL=burger

# Logout/login user or source .profile/.bashrc file

# Clone submodules
git submodule update --init --recursive

# Import local dependencies
cd GAZEBO_Turtlebot3
# Only for the first installation
sudo rosdep init

rosdep update

# Import and build distribution dependencies
rosdep install --from-paths src --ignore-src -r -y -i
colcon build --event-handlers console_direct+ --symlink-install

# Import and build new fresh dependencies
cd src
vcs import . < multi_turtlebot3/turtlebot3.repos
cd ..
rosdep install --from-paths src --ignore-src -r -y -i
colcon build --event-handlers console_direct+ --symlink-install

# Wait.....

# Back to main dir
cd ..
```

## To update dependencies
```bash
cd src
vcs pull
cd ..
colcon build --event-handlers console_direct+ --symlink-install
```

## To run simulation
### Start Gazebo simulation
#### Configuration
It's possible configure the group of robots running in the simulation, changing the file `robots.yaml` (PoC/GAZEBO_Turtlebot3/src/multi_turtlebot3/turtlebot3_simulations/turtlebot3_gazebo/params/robots.yaml), like name, initial position.    
Below an example:   
```yaml
- r1:
  name: 'tb3_1'# name of robot
  x_pose: 4.45 # initial position of X-axis 
  y_pose: 0.5 # initial position of Y-axis
  z_pose: 0.01 # initial position of Z-axis
  roll: 0.0 # initial position of roll
  pitch: 0.0 # initial position of pitch
  yaw: 0.0 # initial position of yaw
  proxy_domain_id: 100 # ROS_DOMAIN_ID used to communicate (if you want) with Proxies
```

#### How to run
```bash
cd PoC
./turtlebot3_run.sh
```

### Out of order usecase
It's possible to simulate the break-down of a robot, decreasing the battery charge to 0, running a bash script.
```bash
cd PoC
./out_of_order.sh $NAMESPACE
```
- $NAMESPACE: name of the robot will be out of order (string)

### To create new SLAM mapping

Open custom simulation (required to avoid namespace support problems):

```bash
export TURTLEBOT3_MODEL=burger
ros2 launch turtlebot3_gazebo turtlebot3_slam_library.launch.py
```

Launch cartographer:

```bash
export TURTLEBOT3_MODEL=burger
ros2 launch turtlebot3_cartographer cartographer.launch.py use_sim_time:=True
```

Run teleop\_keyboard to move and map:

```bash
export TURTLEBOT3_MODEL=burger
ros2 run turtlebot3_teleop teleop_keyboard
```

Save the new map:

```bash
ros2 run nav2_map_server map_saver_cli -f map_filename
```

This operation outputs 2 file map\_filename.yaml and map\_filename.pgm.

### To change battery value

Reset battery to initial value:

```bash
ros2 service call /tb3_1/gazebo_ros_battery/reset battery_services/srv/Reset "{reset: {data: True}}"
```

Set the charge = actual_charge + msg.charge.data:

```bash
ros2 service call /tb3_1/gazebo_ros_battery/set_charge battery_services/srv/SetCharge "{charge: {data: -4.0}}"
```

By default design capacity is 4.0, msg.charge.data should be a negative number.
