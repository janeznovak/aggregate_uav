#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 \"<trajectory_name>\" \"<goal_id>\" (ex: \"traj1 12621\")"
    exit 1
fi

timestamp=$(date +%s%3N)
goal_id="$1-$2"
goal_info="$1"
abort_file="from_user/goals/abort_$timestamp.txt"

echo "ABORT;$goal_id;0.0;0.0;0.0;0.0;$goal_info" > "$abort_file"

echo "File created $abort_file, with goal_id: $goal_id"