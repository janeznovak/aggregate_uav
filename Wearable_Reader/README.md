# Install 

```bash
source Gazebo_Custom_Plugins/wearable_interfaces/install/local_setup.bash
source Robot_Reader/install/local_setup.bash

cd Wearable_Reader
rosdep install --from-paths src --ignore-src -r -y -i
colcon build --event-handlers console_direct+ --symlink-install
source install/local_setup.bash
```

For a quick test:

```
ros2 run wearable_reader wearable_reader actor1 actor1
```

Or more generally

```
ros2 run wearable_reader wearable_reader NAMESPACE WEARABLE_NAME
```
