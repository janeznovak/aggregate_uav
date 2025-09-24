// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

/**
 * @file action_writer.hpp
 * @brief Writer of the action sent from AP
 * 
 */

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <ctime>
#include <regex>
#include <thread>
#include "file_utils.hpp"
#include "common_data.hpp"
#include "poc_config.hpp"

#ifndef ACTION_WRITER_H
#define ACTION_WRITER_H

using namespace std;

namespace action
{
  enum ActionCodeEnum {
    UNDOCK,
    GO_WITH_VEL,
    GO_TO_POSITION,
    ABORT,
    DOCK
  };

  struct ActionData {       
    string action;        
    string goal_code;  
    string robot;
    float pos_x;
    float pos_y;
    float pos_z;
    float orient_w;
    std::chrono::milliseconds milliseconds;
  };

  namespace manager
  {
    class ActionManager {
      private:

      public:
        static void new_action(ActionData data);
    };
  }

  namespace writer
  {
    class ActionWriter {
      private:

      public:
        static void write_dto(ActionData data, string path, string filename);
    };
  }
}

#endif
