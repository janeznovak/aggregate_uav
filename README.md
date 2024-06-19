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

## Clone Repository

To clone the repository with all its submodules, run the following command:

```bash
# using SSH
git clone --recurse-submodules git@github.com:giatorta/fcpp-crazyflie.git

# using HTTPS
git clone --recurse-submodules https://github.com/giatorta/fcpp-crazyflie.git
```

## Install Components

Navigate to the `/Poc` directory and execute the installation script:

```sh
cd Poc
./install_fcpp_crazyflie.sh
cd ..
```

## Execute Demo

### Note:
In the file `Crazyflie/agents.txt`, you will find the initial positions (x, y) of the drones, with each line representing a drone. You can add more drones to the simulation by adding a new line with the coordinates. Ensure that the number of drones listed in this file matches the number configured in ROS2. You can enable the desired drones by setting the `enabled` value to `True` in the following configuration file:

`/Crazyflie/ros2_ws/src/system_launcher/config/crazyfliesConfig.yaml`

### Run Gazebo with its plugin and SITL simulation:

```sh
cd Crazyflie/
./run_gazebo_sitl.sh
```

### Initialize the ROS2 network with all nodes (in another terminal):

```sh
cd PoC
./crazyflie_run.sh
```

### Start the AP Engine (in another terminal):

```sh
cd AP_Engine
./run_engine.sh <number_of_drones>
```

It is important to start the simulation on the AP Engine by pressing the "p" key. You can view the connections between the drones by pressing the "l" key.

### Create a New Goal:

In a new terminal, navigate to the `Storage` directory:

```sh
cd Storage
./create_goal <trajectory_name> <goal_id>
```

This command will create a new goal for the specified trajectory.

### Abort a Goal:

To interrupt a goal, use the following script:

```sh
cd Storage
./abort_goal <trajectory_name> <goal_id>
```

This command will abort the specified goal.
