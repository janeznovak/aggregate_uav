# How to run

## Gazebo simulator with create3

```bash
source /opt/ros/humble/local_setup.bash

vcs import GAZEBO < GAZEBO/src/create3_sim/irobot_create_gazebo/demo.repos
cd GAZEBO
```

```bash
rosdep install --from-path src -yi 
```
or
```bash
rosdep install --from-paths src --ignore-src -r -y -i --os=ubuntu:jammy
```
**Note**: _if even the second command does not work, try the following:_
```bash
sudo apt install libgazebo11
sudo apt install libgazebo-dev
sudo apt install gazebo
```
_and re-run the command._

Then continue as follows.

```bash
colcon build --symlink-install
source install/local_setup.bash

ros2 launch irobot_create_gazebo_bringup create3_gazebo_aws_small.launch.py
```
## Navigator

```bash
colcon build --symlink-install
ros2 run create3_poc_didattico main_node
```
