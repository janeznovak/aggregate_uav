#!/bin/bash
# TODO: Maybe need to modify the script since the create_goal.sh now has goal_id with intiger of the master as well

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 \"<trajectory_name>\" \"<goal_id>\" (ex: \"traj1 12621\") \"<robot_id>\""
    exit 1
fi

timestamp=$(date +%s%3N)
goal_id="$1-$2-$3"
goal_info="$1"
abort_file="from_user/goals/abort_$timestamp.txt"

echo "ABORT;$goal_id;0.0;0.0;0.0;0.0;$goal_info" > "$abort_file"

echo "File created $abort_file, with goal_id: $goal_id"