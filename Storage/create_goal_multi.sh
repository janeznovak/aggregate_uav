#!/bin/bash

# Ask for number of masters
read -p "How many masters do you want? " num_masters

# Validate input is a positive number
if ! [[ "$num_masters" =~ ^[0-9]+$ ]] || [ "$num_masters" -lt 1 ]; then
    echo "Please enter a valid positive number"
    exit 1
fi

# Loop for each master
for ((i=0; i<$num_masters; i++))
do
    echo "Enter details for master $i:"
    read -p "Enter trajectory name: " trajectory_name
    read -p "Enter goal ID: " goal_id
    
    # Create timestamp for unique file name
    timestamp=$(date +%s%3N)
    
    # Construct goal ID and file path
    full_goal_id="$trajectory_name-$goal_id-$i"
    goal_file="from_user/goals/goal_$timestamp.txt"
    
    # Create goal file
    echo "GOAL;$full_goal_id;0.0;0.0;0.0;0.0;$trajectory_name" > "$goal_file"
    echo "File created $goal_file, with goal_id: $full_goal_id"
    
    # Add small delay to ensure unique timestamps
    sleep 0.1
done

echo "All goal files have been created."
