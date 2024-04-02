// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

/**
 * @file common_data.hpp
 * @brief data common to AP and FW.
 */

#ifndef NODES_COMMON_DATA_H_
#define NODES_COMMON_DATA_H_

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "feedback_parser.h"

enum class ProcessingStatus
{
    IDLE, // not used at the moment
    TERMINATING,
    SELECTED
};

//! @brief String representation of a shape.
std::string to_string(ProcessingStatus s);

//! @brief Printing ProcessingStatus.
template <typename O>
O& operator<<(O& o, ProcessingStatus s) {
    o << to_string(s);
    return o;
}

enum class node_type
{
    ROBOT_MASTER,
    ROBOT_SLAVE
};

//! @brief String representation of a node_type.
std::string to_string(node_type nt);

//! @brief Printing node_type.
template <typename O>
O& operator<<(O& o, node_type s) {
    o << to_string(s);
    return o;
}

// TODO: maybe delete this enum?
enum class robot_phase
{
    IDLE,
    GOAL_EXECUTION
};

struct RobotStatus {       
    float pos_x;
    float pos_y;
    float orient_w;
    float battery_percent_charge; //TODO: add other battery fields;
    feedback::GoalStatus goal_status;
    string goal_code;
};

struct InputGoal {       
    std::string action;        
    std::string goal_code;  
    float pos_x;
    float pos_y;
    float orient_w;
    string source;
    int priority;
    string subcode;

        //! @brief Equality operator.
    bool operator==(InputGoal const& m) const {
        return goal_code == m.goal_code;
    }
};

namespace std {
    template <>
    struct hash<InputGoal> {
        //! @brief Produces an hash for a message, combining to and from into a size_t.
        size_t operator()(InputGoal const& m) const {
            return std::hash<std::string>{}(m.goal_code);
        }
    };
}

extern std::unordered_map<std::string,std::vector<RobotStatus>> RobotStatesMap;
extern std::mutex RobotStatesMutex;
extern std::vector<InputGoal> InputGoalList;
extern std::mutex GoalMutex;
#endif