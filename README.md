# FCPP - Crazyflie Project
Components for FCPP - Crazyflie Project

## Documentation

Documentation is available under doc folder:
- *java_server*: documentation of Sensors\_Server
- *ros2*: documentation of components written for ROS2 (humble version)
- *usecase*: some sequence diagrams to depict the relation of components
- *doxygen*: components documented using doxygen (https://www.doxygen.nl/). 

The doxygen documentation can be compiled in /doc/doxygen folder, using
```bash
doxygen doc/Doxyfile 
```
It can be viewed using [/doc/doxygen/html/index.html](/doc/doxygen/html/index.html)

## TL;DR

### Dependencies

#### ROS 2

[ROS 2 installation guide](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debians.html)
  
[ROS 2 DDS tuning](https://docs.ros.org/en/foxy/How-To-Guides/DDS-tuning.html)

```bash
# in ubuntu, before you should enable multiverse repository

# Build deps
apt install python3-colcon-common-extensions python3-vcstool python3-rosdep ros-humble-irobot-create-msgs

# Cyclone DDS
apt install ros-humble-cyclonedds ros-humble-rmw-cyclonedds-cpp
```
#### NODEJS
Choose the best tool for your SO or linux distribution to install last LTS version (14.14+):
- using APT PPA package (for debian-based distributions):
[NODESOURCE installation guide](https://github.com/nodesource/distributions#installation-instructions)
- using NVM (Node Version Manager):
[NVM installation guide](https://codedamn.com/news/nodejs/nvm-installation-setup-guide) 

#### Angular CLI
Install Angular CLI (ng) 13+ locally to project.
[Angular 13 installation guide](https://dev.to/xenxei46/installing-angular-cli-locally-globally-3lei)

#### Maven
Install Maven 3.8.8+ version
[Maven installation guide](https://maven.apache.org/install.html)

### Simulation

To launch a simulation execute from the root of the repository:

```bash
./PoC/turtlebot3_run.sh
```

The script will list the components that need to be compiled to run,
enter the folders and follow compilation instructions.

#### Simulated SLAM

To perform a SLAM of a simulated environment use the following command:

```bash
WORLD_PATH=/home/ws/NODES/use_case_resources/hospital/world.sdf ros2 launch turtlebot3_gazebo turtlebot3_slam.launch.py y_pose:=3.0 x_pose:=1.0
```

This will load the world specified in WORLD\_PATH spawning a Turtlebot3 in position (x\_pose, y\_pose).

On another terminal launch the mapping node:

```bash
TURTLEBOT3_MODEL=burger ros2 launch turtlebot3_cartographer cartographer.launch.py use_sim_time:=True
```

On another terminal launch the keyboard control node:

```bash
TURTLEBOT3_MODEL=burger ros2 run turtlebot3_teleop teleop_keyboard
```

When the map is complete save it using the following command:

```bash
ros2 run nav2_map_server map_saver_cli -f map-filename
```

### AP Engine

To compile and execute the AP engine on a new terminal window:

```bash
cd PoC/AP_Engine
./make.sh build -O ap_engine_embedded
cd bin
AP_NODE_UID=1 AP_NET_BRADDR_IP_ADDR=0.0.0.0 ./run/ap_engine_embedded
```

### Java Server

REQUIRES: openjdk-11, maven

To launch the java server:

```bash
cd Sensors_Server
mvn clean install
mvn spring-boot:run
```

### Angular Server

REQUIRES: nodejs, angular cli

```bash
cd Sensors_Web_App
npm install
# to use specific config: add --configuration $config (library or hospital)
ng serve
```