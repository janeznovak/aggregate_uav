#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 \"<robot_num>\""
    exit 1
fi


robot_num=$1
AP_ROBOT_COUNT=$robot_num  AP_USE_CASE=LOWDISTANCE ./make.sh gui run -O ap_engine  