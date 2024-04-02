# How to run

## HTTP Server Java:
```bash
cd /home/robot/Documenti/projects/NODES/PoC/Sensors_Server
java -jar target/java-sensors-server-0.0.1-SNAPSHOT.jar
```

## Web Application Angular
### Library
```bash
cd /home/robot/Documenti/projects/NODES/PoC/Sensors_Web_App
ng serve --configuration library
```

Open http://localhost:4200/ to view the Web Application

## AP
### Library
To build:
```bash
cd /home/robot/Documenti/projects/NODES/PoC/AP_Engine

./make.sh gui build -O -DAP_COMM_RANGE=5 ap_engine
```

To run:
```bash
cd /home/robot/Documenti/projects/NODES/PoC/AP_Engine      
cd bin
AP_ROUND_PERIOD=0.2 AP_WEARABLE_COUNT=0 AP_ROBOT_COUNT=5 AP_SIMULATOR_OFFSET_X=0.4 ./run/ap_engine
```

To run:
```bash
cd /home/robot/Documenti/projects/NODES/PoC/AP_Engine      
cd bin
AP_ROBOT_PAUSE_SEC=20 AP_SIDE_X=8.0 AP_SIDE_Y=24.0 AP_WEARABLE_COUNT=3 AP_ROBOT_COUNT=4 AP_SIMULATOR_OFFSET_X=0.0 ./run/ap_engine
```

## Simulation
### Library
```bash
cd /home/robot/Documenti/projects/NODES/PoC
./turtlebot3_run.sh
```

### Out of order robot
```bash
# $ROBOT_NAME can be tb3_1, tb3_2 etc...
./out_of_order.sh $ROBOT_NAME
```

# Installation notes

To add the models from `use_case_resources` execute this command from the
root folder of the repository

```bash
export GAZEBO\_MODEL\_PATH=$(pwd):
```
