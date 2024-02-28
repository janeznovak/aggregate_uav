// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

#include "wearable_feedback_parser.h"

/**
 * @file wearable_feedback_parser.cpp
 * @brief Implementation of parsing feedback coming from wearables.
 */

/**** PRIVATE ****/
void add_to_input_status_map(wearable_feedback::WearableFeedbackData data) {
  // lock function
  std::lock_guard<std::mutex> lg(WearableStatesMutex);

  struct WearableStatus status = {.pos_x                 = data.pos_x,
                                  .pos_y                  = data.pos_y,
                                  .goal_status            = data.goal_status,
                                  .goal_code              = data.goal_code
  };

  // create element or append data if the map it's already present
  if (WearableStatesMap.find(data.wearable_code) == WearableStatesMap.end()) {
    WearableStatesMap[data.wearable_code] = {status};
  } else {
    WearableStatesMap[data.wearable_code].push_back(status);
  }  
  // std::cout << "Status added and sent to AP about wearable: "   << data.wearable_code   << endl;
}

/**** PUBLIC ****/
//! @brief String representation of a GoalStatus.
std::string wearable_feedback::to_string(wearable_feedback::GoalStatus s) {
    switch (s) {
        case wearable_feedback::GoalStatus::REACHED:
            return "REACHED";

        case wearable_feedback::GoalStatus::ABORTED:
            return "ABORTED";

        case wearable_feedback::GoalStatus::FAILED:
            return "FAILED";

        case wearable_feedback::GoalStatus::RUNNING:
            return "RUNNING";

        case wearable_feedback::GoalStatus::UNKNOWN:
            return "UNKNOWN";  

        case wearable_feedback::GoalStatus::NO_GOAL:
            return "NO_GOAL";       

        default:
            return "";
    }
}

void wearable_feedback::manager::FeedbackManager::new_line_feedback(std::string content) {
    // std::cout << content << endl;
    auto data = wearable_feedback::parser::FeedbackParser::parse_line(content);
    // std::cout << "wearable_code:"       << data.wearable_code                  << endl;
    // std::cout << "pos_x:"               << data.pos_x                          << endl;
    // std::cout << "pos_y:"               << data.pos_y                          << endl;
    // std::cout << "goal_status:"         << static_cast<int>(data.goal_status)  << endl;
    // std::cout << "goal_code:"           << data.goal_code                      << endl;
    // std::cout << endl;

    add_to_input_status_map(data);
}

wearable_feedback::WearableFeedbackData wearable_feedback::parser::FeedbackParser::parse_line(string line) {
  std::istringstream ss(line);
  std::vector<std::string> line_elements;
  int idx = 0;
  string wearable_code;
  float pos_x                     = NULL_FLOAT_VALUE;
  float pos_y                     = NULL_FLOAT_VALUE;
  int heart_rate_per_minute       = NULL_INT_VALUE;
  float body_temperature          = NULL_FLOAT_VALUE;
  int oxygenation                 = NULL_INT_VALUE;
  GoalStatus goal_status          = wearable_feedback::GoalStatus::UNKNOWN;
  string goal_code;

  for (std::string value; getline(ss, value, COLUMN_DELIMITER);idx++)
  {
    switch (idx)
    {
      case WEARABLE_CODE_WB_FEEDBACK_LINE_POSITION: {
        wearable_code = std::move(value);
        break;
      }
      case POS_X_WB_FEEDBACK_LINE_POSITION: {
        std::string val = std::move(value);
        pos_x = (val != "") ? std::stof(val) : NULL_FLOAT_VALUE;
        break;
      }
      case POS_Y_WB_FEEDBACK_LINE_POSITION: {
        std::string val = std::move(value);
        pos_y = (val != "") ? std::stof(val) : NULL_FLOAT_VALUE;
        break;
      }
      case HEART_RATE_PER_MINUTE_WB_FEEDBACK_LINE_POSITION: {
        std::string val = std::move(value);
        heart_rate_per_minute = (val != "") ? std::stoi(val) : NULL_INT_VALUE;
        break;
      }
      case BODY_TEMPERATURE_WB_FEEDBACK_LINE_POSITION: {
        std::string val = std::move(value);
        body_temperature = (val != "") ? std::stof(val) : NULL_FLOAT_VALUE;
        break;
      }
      case OXYGENATION_WB_FEEDBACK_LINE_POSITION: {
        std::string val = std::move(value);
        oxygenation = (val != "") ? std::stoi(val) : NULL_INT_VALUE;
        break;
      }
      case GOAL_STATUS_WB_FEEDBACK_POSITION: {
        std::string val = std::move(value);
        goal_status = (val != "" && std::stoi(val) >= -1) ? wearable_feedback::GoalStatus(std::stoi(val)) : wearable_feedback::GoalStatus::NO_GOAL;
        break;
      }
      case GOAL_CODE_WB_FEEDBACK_POSITION: {
        goal_code = std::move(value);
        break;
      }
      
      default:
        break;
    }
  }
  struct WearableFeedbackData data = {.wearable_code            = wearable_code,
                                      .pos_x                    = pos_x,
                                      .pos_y                    = pos_y,
                                      .heart_rate_per_minute    = heart_rate_per_minute,
                                      .body_temperature         = body_temperature,
                                      .oxygenation              = oxygenation,
                                      .goal_status              = goal_status,
                                      .goal_code                = goal_code
  };
  return data;
}