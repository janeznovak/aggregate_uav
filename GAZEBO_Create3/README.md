# Installation

## Create PoC Didattico

To compile this package, you need to compile dependencies:
- Navigation_System
- Robot_Reader
- Robot_Writer
- Proxies (if needed)

In the first compilation, you should also initialize ROS2 package manager:
```bash
sudo rosdep init
```
and the git submodules:
```bash
git submodule update --init --recursive
```

Now, you have to source the Navigation_System component before the compilation of GAZEBO_Create3, running:

```bash
source Navigation_System/install/local_setup.bash
```

Then compile all subpackages of GAZEBO_Create3, with:

 ```bash
cd GAZEBO_Create3
rosdep update
rosdep install --from-paths src --ignore-src -r -y -i
colcon build --symlink-install --event-handlers console_direct+
```

OR

To compile only create3_poc_didattico (if you use a headless system):
 - install `ros-humble-ignition-math6-vendor ros-humble-angles ros-humble-control-msgs libboost-all-dev ros-humble-irobot-create-msgs`
 - use command `colcon build --symlink-install --packages-select irobot_create_control irobot_create_description irobot_create_toolbox irobot_create_common_bringup irobot_create_nodes create3_poc_didattico`

# Run

## Simulation
 ```bash
 cd PoC
 ./create3_run.bash $NAMESPACE $X $Y $YAW $USE_PROXIES
 ```
- $NAMESPACE: name of the robot (string)
- $X: initial position of the X-axis of the robot (float)
- $Y: initial position of the Y-axis of the robot (float)
- $YAW: initial position of the yaw (rotation) of the robot (float) 
- $USE_PROXIES: start proxies at boot time (boolean, default: true)

To run various Gazebo instances, you must make sure each of them is using a different port. You can set this with the `GAZEBO_MASTER_URI` environment variable.  
For example: 
```
GAZEBO_MASTER_URI=http://localhost:11546 ./create3_run.bash tb3_1 0.0 1.0 0.0 true
GAZEBO_MASTER_URI=http://localhost:11547 ./create3_run.bash tb3_2 0.0 2.0 0.0 true
```
**Attention**: even if you use this environment variable, the second Gazebo instance will display the models of first Gazebo instance. We don't know why, maybe it's a Gazebo bug.   

## Deployed in board
 ```bash
 cd PoC
 ./create3_board_run.bash $NAMESPACE $X $Y $YAW $USE_PROXIES
 ```
- $NAMESPACE: name of the robot (string)
- $X: initial position of the X-axis of the robot (float)
- $Y: initial position of the Y-axis of the robot (float)
- $YAW: initial position of the yaw (rotation) of the robot (float)
- $USE_PROXIES: start proxies at boot time (boolean, default: true)