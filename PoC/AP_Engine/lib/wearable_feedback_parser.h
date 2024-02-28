// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

/**
 * @file wearable_feedback_parser.h
 * @brief Parse wearable feedback files
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

#ifndef WEARABLE_FEEDBACK_PARSER_H
#define WEARABLE_FEEDBACK_PARSER_H

// CSV positions
#define WEARABLE_CODE_WB_FEEDBACK_LINE_POSITION            0
#define POS_X_WB_FEEDBACK_LINE_POSITION                    1
#define POS_Y_WB_FEEDBACK_LINE_POSITION                    2
#define HEART_RATE_PER_MINUTE_WB_FEEDBACK_LINE_POSITION    3
#define BODY_TEMPERATURE_WB_FEEDBACK_LINE_POSITION         4
#define OXYGENATION_WB_FEEDBACK_LINE_POSITION              5
#define GOAL_STATUS_WB_FEEDBACK_POSITION                   6
#define GOAL_CODE_WB_FEEDBACK_POSITION                     7

using namespace std;

namespace wearable_feedback
{

  enum class GoalStatus
    {
        NO_GOAL = -1,
        REACHED,
        ABORTED,
        FAILED,
        RUNNING,
        UNKNOWN
    };

  //! @brief String representation of a GoalStatus.
  std::string to_string(wearable_feedback::GoalStatus s);

  //! @brief Printing GoalStatus.
  template <typename O>
  O& operator<<(O& o, wearable_feedback::GoalStatus s) {
      o << to_string(s);
      return o;
  }

  struct WearableFeedbackData {       
    string wearable_code;        
    float pos_x;
    float pos_y;
    int heart_rate_per_minute;
    float body_temperature;
    int oxygenation;
    GoalStatus goal_status;
    string goal_code;
  };

  namespace manager
  {
    class FeedbackManager {
      private:

      public:
        static void new_line_feedback(std::string content);
    };
  }

  namespace parser
  {
    class FeedbackParser {
      private:

      public:
        static wearable_feedback::WearableFeedbackData parse_line(string line);
    };
  }
}

#endif
