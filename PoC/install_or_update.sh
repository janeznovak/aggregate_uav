#!/bin/bash

# Prefix used to log
LOG_PREFIX="[INSTALLER]"

POC_FOLDER="./PoC"

# Excluded folders
excluded_folders=("use_case_resources")

# Check if the folder is excluded
is_excluded_folder() {
    local folder="$1"
    for excluded_folder in "${excluded_folders[@]}"; do
        [[ "$folder" == "$excluded_folder" ]] && return 0
    done
    return 1
}

# Log function
log() {
    echo "$LOG_PREFIX $@"
}

log "Starting installation script..."

if [ -d "$POC_FOLDER" ]; then
    WORKING_DIR="$POC_FOLDER"
else
    WORKING_DIR="."
fi

cd "$WORKING_DIR" || exit

log "Use $WORKING_DIR as working directory"

# Install or update custom component
install_component_custom() {
    local component="$1"

    log "Searching for component: $component"

    # Find component case-insensitively
    local component_dir
    component_dir=$(find -L . -maxdepth 1 -type d -iname "*$component*" | head -n 1)

    if [[ -z "$component_dir" ]]; then
        log "Component $component not found."
        return
    fi

    # Resolve symbolic links
    component_dir=$(readlink -f "$component_dir")

    log "Component directory found: $component_dir"

    # Check if folder is to be excluded
    local folder_name
    folder_name=$(basename "$component_dir")
    if is_excluded_folder "$folder_name"; then
        log "Component $component is excluded."
        return
    fi

    # Convert component name to lower case
    local component_lower
    component_lower=$(echo "$component" | tr '[:upper:]' '[:lower:]')

    # Save current directory
    local current_dir
    current_dir=$(pwd)

    cd "$component_dir" || exit

    case "$(basename "$component_lower")" in
        *"ap_engine"*)
            log "Updating AP_Engine..."
            ./make.sh clean
            ./make.sh gui build -O ap_engine
            ;;
        *"sensors_server"*)
            # log "Updating Sensors_Server..."
            # mvn clean install
            ;;
        *"sensors_web_app"*)
            # log "Updating Sensors_Web_App..."
            # npm install --legacy-peer-deps
            # ng build
            ;;
        *"gazebo_turtlebot3"*)
            # log "Updating Gazebo_Turtlebot3..."
            # cd src || exit
            # vcs import . < multi_turtlebot3/turtlebot3.repos
            # cd ..
            # source ../Navigation_System/install/local_setup.bash
            # rosdep install --from-paths src --ignore-src -r -y -i --os="$OS"
            # colcon build --symlink-install
            ;;
        *"gazebo_custom_plugin"*)
            # log "Updating Gazebo_Custom_Plugins..."
            # cd wearable_interfaces
            # log "Installing/updating component Gazebo_Custom_Plugins/wearable_interfaces"
            # colcon build --symlink-install
            # source install/local_setup.bash
            # cd ..
            # install_ros2_component
1            ;;
        
        # All Ros2 Components
        *"crazyflie"*)
            log "Updating crazyflie-lib-python..."
            cd crazyflie-lib-python
            pip install -e .
            cd ..
            log "Updating Crazyflie Firmware..."
            cd crazyflie-firmware
            rm -r sitl_make/build
            mkdir -p sitl_make/build && cd $_
            cmake ..
            make all
            cd ../../../

            log "Updating CrazySwarm components..."
            cd ros2_ws/
            install_crazyswarm_component
            source install/local_setup.bash
        ;;


        *)
            # Update with standard logic
            log "Updating ROS2 component: $component_dir"
            install_ros2_component
            source install/local_setup.bash
            ;;
    esac

    log "Installed component successfully: $component_lower"

    # Go back to the parent directory
    cd "$current_dir" || exit
}

# Install or update all components
install_all_components() {
    local navigation_system_installed=false

    # Check if Navigation_System is present and install it first if it is
    if [ -d "Navigation_System" ]; then
        log "Installing/updating Navigation_System"
        install_component_custom "Navigation_System"
        navigation_system_installed=true
    fi

    # Install/update other components
    for component_dir in */; do
        # Skip Navigation_System if it was installed first
        if [ "$navigation_system_installed" = true ] && [ "$component_dir" = "Navigation_System/" ]; then
            continue
        fi
        log "Installing/updating component: ${component_dir%/}"
        install_component_custom "${component_dir%/}"
    done
}

# Install or update ROS2 component
install_ros2_component() {
    log "Clearing build directories..."
    find . -type d \( -name "build" -o -name "install" -o -name "log" \) -exec rm -rf {} +
    rosdep install --from-paths src --ignore-src -r -y -i --os="$OS"
    log "Build..."
    colcon build --symlink-install
    source install/local_setup.bash
}

install_crazyswarm_component() {
    log "Build..."
    rm -rf build/ log/ install/
    rosdep install --from-paths src --ignore-src -r -y -i --os="$OS"
    colcon build --symlink-install --cmake-args -DCMAKE_BUILD_TYPE=Release
    source install/local_setup.bash
}

# Update submodules
log "Updating submodules..."
git submodule update --init --recursive
log "Submodules updated."

# Initialize rosdep only if it hasn't been initialized before
if [ ! -f "/etc/ros/rosdep/sources.list.d/20-default.list" ]; then
    log "Initializing rosdep..."
    sudo rosdep init
    log "Rosdep initialized."
fi

# Declare the OS used
OS="ubuntu:jammy"
log "Operating system: $OS"

# Select component to update
if [ $# -eq 0 ]; then
    log "No components specified. Updating all components."
    install_all_components
else
    for component in "$@"; do
        log "Installing/updating component: $component"
        install_component_custom "$component"
    done
fi