#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 \"<robot_num>\""
    exit 1
fi


robot_num=$1
AP_ROBOT_COUNT=$robot_num  ./make.sh gui run -DAP_USE_CASE=HIGHDISTANCE -O ap_engine  
