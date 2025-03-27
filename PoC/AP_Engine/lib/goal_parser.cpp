// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

#include "goal_parser.h"

/**** PRIVATE ****/
void add_to_input_goal_list(goal::GoalData data) {
  // lock function
  std::lock_guard<std::mutex> lg(GoalMutex);

  struct InputGoal goal = { .action = data.action,
                            .goal_code = data.goal_code,
                            .pos_x = data.pos_x,
                            .pos_y = data.pos_y,
                            .pos_z = data.pos_z,
                            .orient_w = data.orient_w,
                            .source = data.source,
                            .priority = data.priority,
                            .subcode = data.subcode
  };
  InputGoalList.push_back(goal);
  std::cout << "Goal " << data.action << " added and sent to AP about robot: " << data.goal_code << endl;
}

/**** PUBLIC ****/
// gets a line and parse it. It gets the content from the new_file_callback in poc_file_watcher.cpp
void goal::manager::GoalManager::new_line_goal(std::string content) {
  std::cout << content << endl;
  auto data = goal::parser::GoalParser::parse_line(content);
  std::cout << "action:" << data.action << endl;
  // in the goal code there is the trajectory-code, now there is also the robot id, seperated by a -
  std::cout << "goal_code:" << data.goal_code << endl;
  if (data.pos_x != NULL_FLOAT_VALUE)
    std::cout << "pos_x:" << data.pos_x << endl;
  if (data.pos_y != NULL_FLOAT_VALUE)
    std::cout << "pos_y:" << data.pos_y << endl;
  if (data.pos_z != NULL_FLOAT_VALUE)
    std::cout << "pos_z:" << data.pos_z << endl;
  if (data.orient_w != NULL_FLOAT_VALUE)
    std::cout << "orient_w:" << data.orient_w << endl;
  std::cout << "source:" << data.source << endl;
  if (data.priority != NULL_INT_VALUE)
    std::cout << "priority:" << data.priority << endl;
  std::cout << "subcode:" << data.subcode << endl;
  std::cout << endl;

  add_to_input_goal_list(data);
}

goal::GoalData goal::parser::GoalParser::parse_line(string line) {
  std::istringstream ss(line);
  std::vector<std::string> line_elements;
  int idx = 0;
  string action;
  string goal_code;
  string source;
  float pos_x = NULL_FLOAT_VALUE;
  float pos_y = NULL_FLOAT_VALUE;
  float pos_z = NULL_FLOAT_VALUE;
  float orient_w = NULL_FLOAT_VALUE;
  int priority = NULL_INT_VALUE;
  string subcode;
  for (std::string value; getline(ss, value, COLUMN_DELIMITER);idx++)
  {
    switch (idx) {
      case ACTION_GOAL_LINE_POSITION: {
        action = std::move(value);
        break;
      }
      case GOAL_CODE_GOAL_LINE_POSITION: {
        goal_code = std::move(value);
        break;
      }
      case POS_X_GOAL_LINE_POSITION: {
        std::string val = std::move(value);
        pos_x = (val != "") ? std::stof(val) : NULL_FLOAT_VALUE;
        break;
      }
      case POS_Y_GOAL_LINE_POSITION: {
        std::string val = std::move(value);
        pos_y = (val != "") ? std::stof(val) : NULL_FLOAT_VALUE;
        break;
      }
      case POS_Z_GOAL_LINE_POSITION: {
        std::string val = std::move(value);
        pos_z = (val != "") ? std::stof(val) : NULL_FLOAT_VALUE;
        break;
      }
      case ORIENT_W_GOAL_LINE_POSITION: {
        std::string val = std::move(value);
        orient_w = (val != "") ? std::stof(val) : NULL_FLOAT_VALUE;
        break;
      }
      case SOURCE_GOAL_LINE_POSITION: {
        source = std::move(value);
        break;
      }
      case PRIORITY_GOAL_LINE_POSITION: {
        std::string val = std::move(value);
        priority = (val != "") ? std::stoi(val) : NULL_INT_VALUE;
        break;
      }
      case SUBCODE_GOAL_LINE_POSITION: {
        subcode = std::move(value);
        break;
      }
      default:
        break;
    }
  }
  struct GoalData data = { .action = action,
                          .goal_code = goal_code,
                          .pos_x = pos_x,
                          .pos_y = pos_y,
                          .pos_z = pos_z,
                          .orient_w = orient_w,
                          .source = source,
                          .priority = priority,
                          .subcode = subcode
  };
  return data;
}
