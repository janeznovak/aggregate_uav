#!/bin/bash

LOG_PREFIX="[INSTALL]"

# Log function
log() {
    echo "$LOG_PREFIX $@"
}

# Function to build a package with make.sh
build_make_sh_package() {
    local package_dir=$1
    local build_options=$2
    log "Building $package_dir..."
    cd $package_dir
    ./make.sh clean
    ./make.sh $build_options
    cd ../..
}

# Function to build a colcon package
build_colcon_package() {
    local package_dir=$1
    log "Building $package_dir..."
    cd $package_dir
    colcon build
    . install/local_setup.bash 
    cd ..
}

# Function to build Crazyflie lib python
build_crazyflie_lib_python() {
    log "Building crazyflie-lib-python..."
    cd Crazyflie/crazyflie-lib-python
    pip install -e .
    cd ../..
}

# Function to build Crazyflie firmware
build_crazyflie_firmware() {
    log "Building Crazyflie Firmware..."
    cd Crazyflie/crazyflie-firmware
    rm -r sitl_make/build
    mkdir -p sitl_make/build && cd $_
    cmake ..
    make all
    cd ../../../..
    
}

# Function to build CrazySwarm components
build_crazyswarm_components() {
    log "Building CrazySwarm components..."
    cd Crazyflie/ros2_ws/src
    colcon build
    . install/local_setup.bash
    cd ../../..
}

# Main function to call all build functions
main() {
    # build_make_sh_package "AP_Engine" "gui build -O ap_engine"
    build_colcon_package "Robot_Reader"
    build_colcon_package "Robot_Writer"
    #build_colcon_package "Navigation_System"
    #build_crazyflie_lib_python
    build_crazyflie_firmware
    build_crazyswarm_components
}

# Execute the main function
main
