// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include "lib/action_writer.hpp"
#include "lib/poc_file_watcher.hpp"

using namespace std;

int main() {
  // action::ActionData action_data = {
  //   .action = "GOAL",
  //   .goal_code = "GOAL_1",
  //   .robot = "ROBOT1",
  //   .pos_x = 1.0,
  //   .pos_y = 2.0,
  //   .orient_w = 3.0
  // };
  // action::manager::ActionManager::new_action(action_data);

  // FILE WATCHER
  poc_file_watcher poc_file_watcher;
  poc_file_watcher.run_watch_files();
}