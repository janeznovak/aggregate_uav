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
./Poc/turtlebot3_run.bash
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

## Use Create3 in real environment
###  Setup board for create3
#### *Only for SLAM and NAV2*

##### On board
- Install ROS2 Humble dependencies following instructions in this [README](#ros-2)
- Install Cyclone DDS to have better performance
- Source ROS2 dependencies. For example, you can add at the bottom of `~/.profile` the following snippet:
  ```bashrc
  source /opt/ros/humble/setup.bash
  ```
- Disable power management of your wifi adapter. At boot time, create or add a script in /etc/rc.local with:
    ```bash
    # substitute $WIFI_ADAPTER with your wifi board
    iw $WIFI_ADAPTER set power_save off
    ```
- Configure ethernet network on minipc, setting manual IP `192.168.186.3/24`.
  If you are using netplan, you can use this snippet (substitute $ETH_ADAPTER):
  ```yaml
  ethernets:
    renderer: networkd
    $ETH_ADAPTER:
      dhcp4: false
      optional: true
      addresses: [192.168.186.3/24]
  ```
- Configure wifi network on minipc, setting manual IP or DCHP leasing
- Check IP connection between:
  - Create3 and minipc (via ping ethernet)
  - minipc and laptop (via ping wifi)
- Edit the file [cyclone-dds-interface-select-minipc.xml](/config/cyclone-dds-interface-select-minipc.xml)` with with your configuration:
  - checking wifi and ethernet interfaces of minipc
  - IP of your laptop /CycloneDDS/Domain/Discovery/Peers
- Check ROS2 connection between Create3 and minipc (via `ros2 topic list`) 
- Follow instructions about [GAZEBO_Create3](/GAZEBO_Create3/README.md#deployed-in-board) to deploy Create Poc Didattico on board, installing:
  - [Navigation_System](/Navigation_System/README.md)
- Copy and edit the script [config.vars](/config/config.vars) in your ~HOME
- Source at login the previous script. For example, you can add at the bottom of `~/.profile` the following snippet:
  ```bashrc
  if [ -f "$HOME/.vars" ]; then
  . "$HOME/.vars"
  fi
  ```
- Run undock on Create3
- Run SLAM and save the map, following the instructions on [README](/Navigation_System/README.md#run-slam)
- Run Rviz to send goal to nav2, following the instructions on [README](/Navigation_System/README.md#run-navigation)

##### On Create3
- Configure Create3 using its Web Application, like below:
  - In `Application/Configuration`:
    - Set `ROS 2 Domain ID` according to the value set in $HOME/.vars
    - Leave empty `ROS 2 Namespace`
    - Set `RMW_IMPLEMENTATION` to `rmw_cyclonedds_cpp`
    - Set `Application ROS2 Parameter File` to:
      ```yaml
        /motion_control:
          ros__parameters:
          safety_override: "backup_only"
          wheel_accel_limit: 50
      ```
  - In `Beta Feature/Edit ntp.conf`, add the IP address of your laptop (check the ntp configuration of your laptop)
  - In `Beta Feature/RMW Profile Override`, set the content of the file [cyclone-dds-interface-select-robot.xml](/config/cyclone-dds-interface-select-robot.xml)

##### On Laptop
- Install or configure NTPD to serve client connection of board and create3
- Edit the file [cyclone-dds-interface-select-laptop.xml](/config/cyclone-dds-interface-select-laptop.xml)` with your configuration:
  - wifi interface of laptop
  - IP of your minipc /CycloneDDS/Domain/Discovery/Peers
- Add in your .profile:
  ```bash
  export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
  # change path according with your OS
  export CYCLONEDDS_URI=~/config/cyclone-dds-interface-select-laptop.xml
_______________

#### *Complete setup*

##### On board
- Follow instructions about [GAZEBO_Create3](/GAZEBO_Create3/README.md#deployed-in-board) to deploy Create Poc Didattico on board, installing:
  - [Robot_Reader](/Robot_Reader/README.md)
  - [Robot_Writer](/Robot_Writer/README.md)
  - [Proxies](/Proxies/README.md)
- Run ROS2 components, executing:
  ```bash
  # $X $Y $YAW are 0.0 if you are using NAV2 
  ./PoC/create3_board_run.bash $ROBOT_NAME $X $Y $YAW
  ```
- Run AP Engine, following instructions of [README](/PoC/AP_Engine/README.MD#deployment-on-single-node-using-udp-for-communication)
- Run undock on Create3

##### On laptop
- Install Angular CLI, Maven, Java dependencies
- Install [Sensors_Server](/Sensors_Server/README.md)
- Install [Sensors_Web_App](/Sensors_Web_App/README.md)
- Add in your .profile:
  ```bash
  # path of your maven installation
  M2_HOME='/opt/maven/apache-maven-3.8.8' 
  PATH="$M2_HOME/bin:$PATH"
  ```
- Run Sensors_Web_App, following instructions on [README](/Sensors_Web_App/README.md)
- Open browser and navigate to http://localhost:4200/
- Run Sensors_Server, following instructions on [Sensors_Server](/Sensors_Web_App/README.md)
- Run Server Proxies, executing:
  ```bash
  ./PoC/server_proxies_run.bash
  ```
