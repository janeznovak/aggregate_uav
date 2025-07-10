#!/bin/bash

function cleanup() {
    pkill -x cf2
    pkill -9 ruby
}

function spawn_crazyflie() {
    MODEL=$1
    N=$2 # Cf ID
    X=$3 # spawn x position
    Y=$4 # spawn y position
    X=${X:=$X}
    Y=${Y:=$Y}
    
    SUPPORTED_MODELS=("crazyflie" "crazyflie_thrust_upgrade")
    if [[ " ${SUPPORTED_MODELS[*]} " != *"$MODEL"* ]]; then
        echo "ERROR: Currently vehicle model $MODEL is not supported!"
        echo " Supported Models: [${SUPPORTED_MODELS[@]}]"
        trap "cleanup" SIGINT SIGTERM EXIT
        exit 1
    fi
    
    working_dir="$build_path/$n"
    [ ! -d "$working_dir" ] && mkdir -p "$working_dir"
    pushd "$working_dir" &>/dev/null
    
    set --
    set -- ${@} ${src_path}/tools/crazyflie-simulation/simulator_files/gazebo/launch/jinja_gen.py
    set -- ${@} ${src_path}/tools/crazyflie-simulation/simulator_files/gazebo/models/${MODEL}/model.sdf.jinja
    set -- ${@} ${src_path}/tools/crazyflie-simulation/simulator_files/gazebo
    set -- ${@} --cffirm_udp_port $((19950+${N}))
    set -- ${@} --cflib_udp_port $((19850+${N}))
    set -- ${@} --cf_id $((${N}))
    set -- ${@} --cf_name cf
    set -- ${@} --output-file /tmp/${MODEL}_${N}.sdf
    python3 ${@}
    
    echo "Spawning ${MODEL}_${N} at ${X} ${Y}"
    gz service -s /world/${world}/create --reqtype gz.msgs.EntityFactory --reptype gz.msgs.Boolean --timeout 300 --req 'sdf_filename: "/tmp/'${MODEL}_${N}'.sdf", pose: {position: {x:'${X}', y:'${Y}', z: 0.5}}, name: "'${MODEL}_${N}'", allow_renaming: 1'
    
    echo "starting instance $N in $(pwd)"
    $build_path/cf2 $((19950+${N})) > out.log 2> error.log &
    popd &>/dev/null
}

function spawn_custom_robot() {
    MODEL=$1
    N=$2
    X=$3
    Y=$4
    X=${X:=$X}
    Y=${Y:=$Y}
    
    echo "Spawning ${MODEL}_${N} at ${X} ${Y}"
    gz service -s /world/${world}/create --reqtype gz.msgs.EntityFactory --reptype gz.msgs.Boolean --timeout 300 --req 'sdf_filename: "robot.sdf", pose: {position: {x:'${X}', y:'${Y}', z: 0.5}}, name: "'${MODEL}_${N}'", allow_renaming: 1'
}

if [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    echo "Usage: $0 [-n <num_vehicles>] [-m <vehicle_model>] [-w <world>] [-s <script>]"
    echo "-s flag is used to script spawning vehicles e.g. $0 -s crazyflie:3"
    exit 1
fi

while getopts n:m:w:s:t:l: option
do
    case "${option}"
    in
        m) VEHICLE_MODEL=${OPTARG};;
        w) WORLD=${OPTARG};;
        s) SCRIPT=${OPTARG};;
        t) TARGET=${OPTARG};;
        l) LABEL=_${OPTARG};;
    esac
done

world=${WORLD:=crazysim_default}
target=${TARGET:=cf2}
vehicle_model=${VEHICLE_MODEL:="crazyflie"}
export CF2_SIM_MODEL=gz_${vehicle_model}

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
src_path="$SCRIPT_DIR/crazyflie-firmware"
build_path=${src_path}/sitl_make/build

echo "killing running crazyflie firmware instances"
pkill -x cf2 || true
sleep 1

source ${src_path}/tools/crazyflie-simulation/simulator_files/gazebo/launch/setup_gz.bash ${src_path} ${build_path}

echo "Starting gazebo"
gz sim -s -r worlds/${world}.sdf -v 3 &
sleep 3

n=0

if [ -z ${SCRIPT} ]; then
    while IFS= read -r line || [ -n "$line" ]; do
        fields=($(printf "%s" "$line"|cut -d',' --output-delimiter=' ' -f1-))
        robot_type=${fields[2]:-"crazyflie"}  # If field[2] is empty, default to crazyflie
        
        if [ "$robot_type" = "crazyflie" ]; then
            spawn_crazyflie ${vehicle_model} $(($n)) ${fields[0]} ${fields[1]}
        else
            spawn_custom_robot ${robot_type} $(($n)) ${fields[0]} ${fields[1]}
        fi
        n=$(($n + 1))
    done < "agents.txt"
else
    IFS=,
    for target in ${SCRIPT}; do
        target="$(echo "$target" | tr -d ' ')" #Remove spaces
        target_vehicle=$(echo $target | cut -f1 -d:)
        target_number=$(echo $target | cut -f2 -d:)
        target_x=$(echo $target | cut -f3 -d:)
        target_y=$(echo $target | cut -f4 -d:)
        
        if [ $n -gt 255 ]; then
            echo "Tried spawning $n vehicles. The maximum number of supported vehicles is 255"
            exit 1
        fi
        
        m=0
        while [ $m -lt ${target_number} ]; do
            export CF2_SIM_MODEL=gz_${target_vehicle}
            spawn_crazyflie ${target_vehicle}${LABEL} $(($n)) $target_x $target_y
            m=$(($m + 1))
            n=$(($n + 1))
        done
    done
fi

trap "cleanup" SIGINT SIGTERM EXIT

echo "Starting gazebo gui"
gz sim -g
# gz sim -s
