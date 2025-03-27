#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 \"<robot_num>\"" >&2
    exit 1
fi
robot_num=$1
# AP_ROBOT_COUNT=$robot_num AP_USE_CASE=0 ./make.sh gui run -O ap_engine
AP_USE_CASE=0 ./make.sh gui build -O ap_engine &&
cd bin &&
AP_ROBOT_COUNT=$robot_num ./run/ap_engine >> ../ap_output.txt 2>&1