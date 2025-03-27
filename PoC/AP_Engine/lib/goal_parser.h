// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

/**
 * @file goal_parser.h
 * @brief Parse goal files
 */

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <sstream>
#include "common_data.hpp"
#include "poc_config.hpp"

#ifndef GOAL_PARSER_H
#define GOAL_PARSER_H

 // CSV positions
#define ACTION_GOAL_LINE_POSITION 0
#define GOAL_CODE_GOAL_LINE_POSITION 1
#define POS_X_GOAL_LINE_POSITION 2
#define POS_Y_GOAL_LINE_POSITION 3
#define POS_Z_GOAL_LINE_POSITION 4
#define ORIENT_W_GOAL_LINE_POSITION 5
#define SOURCE_GOAL_LINE_POSITION 6
#define PRIORITY_GOAL_LINE_POSITION 7
#define SUBCODE_GOAL_LINE_POSITION 8

using namespace std;

namespace goal
{
  struct GoalData
  {
    string action;
    string goal_code;
    float pos_x;
    float pos_y;
    float pos_z;
    float orient_w;
    string source;
    int priority;
    string subcode;
  };

  namespace manager
  {
    class GoalManager
    {
    private:
    public:
      static void new_line_goal(std::string content);
    };
  }

  namespace parser
  {
    class GoalParser
    {
    private:
    public:
      static goal::GoalData parse_line(string line);
    };
  }
}

string get_robot_id_from_goal_code(string goal_code);
#endif
