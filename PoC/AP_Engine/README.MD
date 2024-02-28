# First install

```sh
# download fcpp
git submodule init
git submodule update 
```

# Configuration (common options)
Export variables:
- `AP_ROBOT_PREFIX`: prefix used to compute robot node (ex: tb3_ -> tb3_1, tb3_2, tb3_3 ecc...) (DEFAULT: tb3_)
- `AP_WEARABLE_PREFIX`: prefix used to compute wearable node (ex: wb_ -> wb_1, wb_2, wb_3 ecc...) (DEFAULT: wb_)
- `AP_ROUND_PERIOD`: period of a round in seconds (DEFAULT: 0.2)
- `AP_SIDE_X`: size of x-axis where robots run (DEFAULT: 5.3)
- `AP_SIDE_Y`: size of x-axis where robots run (DEFAULT: 8.0)
- `AP_ALG_USED`: type of algorithm used in AP Engine (DEFAULT: LAZY)
- `AP_CUSTOM_GRAPH_DIAMETER`: custom graph diameter used to gossip information between nodes (DEFAULT: 2 * min(n_robots, 2 * (round of max_distance / communication range)))
- `AP_ROBOT_PAUSE_SEC`: pause (in seconds) of a robot from last reached goal before to compute other goals (DEFAULT: 0)
- `AP_SIMULATOR_OFFSET_X`: offset in x-axis used to show position of node in simulator (DEFAULT: 0.4)
- `AP_SIMULATOR_OFFSET_Y`: offset in y-axis used to show position of node in simulator (DEFAULT: 0.0)
- `AP_BG_IMAGE`: filename of a image file inside the textures folder to use as simulation background (DEFAULT: library.jpg)
- `AP_ROBOT_PAUSE_SEC`: the robot stops for AP_ROBOT_PAUSE_SEC seconds after reached a goal (DEFAULT: 0)

CMake variables:
- `AP_ENGINE_DEBUG`: enable/disable log of AP_Engine (DEFAULT: false). To use, add to build argument `-DAP_ENGINE_DEBUG=false`. To change config, you should run `./make.sh clean` before.
- `AP_COMM_RANGE`: max communication range of a node (DEFAULT: 5). To use, add to build argument `-DAP_COMM_RANGE=5`. To change config, you should run `./make.sh clean` before.
- `AP_RETAIN_SEC`: retain time of messages of a node (DEFAULT: 2). To use, add to build argument `-ADP_RETAIN_SEC=2`. To change config, you should run `./make.sh clean` before.

# Simulation

## Configuration
Export variables:
- `AP_ROBOT_COUNT`: number of robots running in the simulation (DEFAULT: 5).
- `AP_WEARABLE_COUNT`: number of wearables running in the simulation (DEFAULT: 0).

## Build
```sh
./make.sh gui build ap_engine 
```

## Run
```sh
./make.sh gui run -O ap_engine 
```

or

```sh
cd bin
./run/ap_engine
```

# Deployment on single node, using UDP for communication

## Configuration
Export variables:
- `AP_ROBOT_COUNT`: number of robots present in the usecase (DEFAULT: 5). 
- `AP_NODE_UID`: uid of the node in the deployment mode (DEFAULT: 1)
- `AP_NET_BRADDR_PORT`: port of the communication intra-node using UDP (DEFAULT: 10000)
- `AP_NET_BRADDR_IP`: IP broadcast used in communication intra-node (DEFAULT: 192.168.1.255)
- `AP_RECV_TIMEOUT_MS`: Timeout used to lock socket when is receiving data (DEFAULT: 20)
- `AP_EMBEDDED_NODE_KIOSK`: node that is responsible for managing the start of new goals (DEFAULT: 1)

## Build
```sh
./make.sh build -O ap_engine_embedded
```

## Run
```sh
./make.sh run -O ap_engine_embedded 
```

or

```sh
cd bin
./run/ap_engine_embedded
