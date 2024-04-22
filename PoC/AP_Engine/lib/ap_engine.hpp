// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

/**
 * @file ap_engine.hpp
 * @brief AP code for the engine.
 *
 * It's used for simulation and embedded deployment.
 */

#ifndef NODES_AP_ENGINE_H_
#define NODES_AP_ENGINE_H_

#include <string>
#include <ctime>
#include <cmath>

#include "lib/poc_config.hpp"
#include "lib/fcpp.hpp"

#include "lib/common_data.hpp"
#include "lib/ap_engine_setup.hpp"
#include "lib/action_writer.hpp"

namespace fcpp
{

    namespace coordination
    {

        using namespace fcpp::coordination::tags;
        using namespace fcpp::option::data;
        using spawn_result_type = std::unordered_map<goal_tuple_type, times_t, fcpp::common::hash<goal_tuple_type>>;

        // UTILS AP

        //! @brief Change color of node, passing in argument
        FUN void change_color(ARGS, fcpp::color color) {
            CODE
                node.storage(left_color{}) = fcpp::color(color);
            node.storage(right_color{}) = fcpp::color(color);
            node.storage(node_color{}) = fcpp::color(color);
        }

        //! @brief Blink color of node
        FUN void blink_computing_color(ARGS, int n_round) {
            CODE
                fcpp::color computing_color = fcpp::color(fcpp::coordination::computing_colors[n_round % 2]);
            change_color(CALL, computing_color);
        }

        //! @brief Get robot name from AP node_uid
        FUN string get_real_robot_name(ARGS, device_t node_uid) {
            CODE
                return get_robot_name(ROBOTS, static_cast<int>(node_uid));
        }

        //! @brief Update in the storage the tag "node_external_status_update_time"
        FUN void update_last_goal_update_time(ARGS) {
            CODE
                std::time_t now = std::time(nullptr);
            std::asctime(std::localtime(&now));
            node.storage(node_external_status_update_time{}) = now;
        }

        //! @brief Check if is passed "diff_time_ms" milliseconds from last REACHED change status
        FUN bool has_ms_passed_from_last_goal_update(ARGS, long diff_time_ms) {
            CODE
                std::time_t now = std::time(nullptr);
            std::chrono::milliseconds millis_to_check(diff_time_ms);
            std::time_t stored_time = node.storage(node_external_status_update_time{});

            std::chrono::milliseconds millis_computed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::from_time_t(now) - std::chrono::system_clock::from_time_t(stored_time));
            return millis_computed >= millis_to_check;
        }

        //! @brief Add current goal to computing map
        FUN void add_goal_to_computing_map(ARGS, goal_tuple_type const& g) {
            CODE
                node.storage(node_process_computing_goals{})[common::get<goal_code>(g)] = g;
        }

        //! @brief Remove current goal to computing map
        FUN void remove_goal_from_computing_map(ARGS, std::string goal_code) {
            CODE
                node.storage(node_process_computing_goals{}).erase(goal_code);
        }

        //! @brief Send "stop" command to robot and change "node_process_status" after it
        FUN void send_stop_command_to_robot(ARGS, string action, device_t node_uid, goal_tuple_type const& g, ProcessingStatus ps) {
            CODE
                std::string robot_chosen = get_real_robot_name(CALL, node_uid);
            std::cout << "ROBOT_MASTER " << robot_chosen << " is chosen for receiving ABORT command for goal " << common::get<goal_code>(g) << endl;
            std::cout << endl;

            action::ActionData action_data = {
                .action = action,
                .goal_code = common::get<goal_code>(g),
                .robot = robot_chosen,
                .pos_x = common::get<goal_pos_x>(g),
                .pos_y = common::get<goal_pos_y>(g),
                .pos_z = common::get<goal_pos_z>(g),
                .orient_w = common::get<goal_orient_w>(g)
            };

            action::manager::ActionManager::new_action(action_data);

            // clear goal info if in idle
            if (ProcessingStatus::IDLE == ps) {
                node.storage(node_process_goal{}) = "";
            }

            // clear processing status
            node.storage(node_process_status{}) = ps;
        }

        // STATE MACHINE

        //! @brief Manage state machine when battery is discharged
        FUN void manage_battery_discharged_node(ARGS) {
            CODE
                change_color(CALL, fcpp::color(fcpp::coordination::discharged_color));
        }

        //! @brief Manage state machine when robot is running a goal
        FUN void manage_running_goal_status(ARGS) {
            CODE
                change_color(CALL, fcpp::color(fcpp::coordination::running_color));
        }

        //! @brief Manage state machine when robot has reached the goal
        FUN void manage_reached_goal_status(ARGS) {
            CODE
                fcpp::color new_color;
            // and previous state is NOT REACHED: new color is "reached" and deletes goal storage
            if (feedback::GoalStatus::REACHED != node.storage(node_external_status{})) {
                new_color = fcpp::color(fcpp::coordination::reached_goal_color);
                // set to terminating processing status
                node.storage(node_process_status{}) = ProcessingStatus::TERMINATING;
                // update time 
                update_last_goal_update_time(CALL);

                // and if process node is SELECTED, then AP has selected new node and simulation is ready to start    
            }
            else if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED) {
                new_color = fcpp::color(fcpp::coordination::running_color);

                // otherwise: new color is "reached"
            }
            else {
                new_color = fcpp::color(fcpp::coordination::reached_goal_color);
            }
            change_color(CALL, new_color);
        }

        //! @brief Manage state machine when robot reported an UNKNOWN error
        FUN void manage_unknown_goal_status(ARGS) {
            CODE
                fcpp::color new_color;
            // and if process node is SELECTED, then AP has selected new node and simulation is ready to start    
            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED) {
                new_color = fcpp::color(fcpp::coordination::running_color);
            }
            else {
                new_color = fcpp::color(fcpp::coordination::idle_color);
                // update time 
                update_last_goal_update_time(CALL);
            }
            change_color(CALL, new_color);
        }

        //! @brief Manage state machine when robot reported an abort of the current goal
        FUN void manage_aborted_goal_status(ARGS) {
            CODE
                fcpp::color new_color;
            // and if process node is SELECTED, then AP has selected new node and simulation is ready to start    
            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED) {
                new_color = fcpp::color(fcpp::coordination::running_color);
            }
            else {
                new_color = fcpp::color(fcpp::coordination::aborted_goal_color);
                // update time 
                update_last_goal_update_time(CALL);
            }
            change_color(CALL, new_color);
        }

        //! @brief Manage state machine when robot reported a FAILED error
        FUN void manage_failed_goal_status(ARGS) {
            CODE
                fcpp::color new_color;
            // and previous state is not FAILED (first time of FAILED status): change internal status to IDLE
            if (feedback::GoalStatus::FAILED != node.storage(node_external_status{})) {
                new_color = fcpp::color(fcpp::coordination::failed_goal_color);
                // resetting processing status
                node.storage(node_process_status{}) = ProcessingStatus::IDLE;
                // update time  
                update_last_goal_update_time(CALL);

                // or previous state is FAILED (it's NOT the first time of FAILED status)
            }
            else {
                // and if process node is SELECTED, then AP has selected new node and simulation is ready to start    
                if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED) {
                    new_color = fcpp::color(fcpp::coordination::running_color);
                }
                else {
                    new_color = fcpp::color(fcpp::coordination::failed_goal_color);
                }
            }
            change_color(CALL, new_color);
        }

        //! @brief Manage state machine when robot has not yet executed a goal
        FUN void manage_no_goal_status(ARGS) {
            CODE
                fcpp::color new_color;
            // and if process node is SELECTED, then AP has selected new node and simulation is ready to start    
            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED) {
                new_color = fcpp::color(fcpp::coordination::running_color);
            }
            else {
                new_color = fcpp::color(fcpp::coordination::idle_color);
            }
            change_color(CALL, new_color);
        }

        // AP PROCESS

        //! @brief A robot has reached a goal and now try to terminate the process
        FUN process_tuple_type ends_processed_goal(ARGS, process_tuple_type& p, goal_tuple_type const& g, status* s) {
            CODE

                std::cout << "Robot " << node.uid << " is trying to terminate goal " << common::get<goal_code>(g) << endl;
            std::cout << endl;

            *s = status::terminated_output; // stop propagation
            return p;
        }

        //! @brief A robot has discharged the battery, so AP send a stop command
        FUN void battery_discharged_when_it_is_running(ARGS, process_tuple_type& p, goal_tuple_type const& g, status* s) {
            CODE
                send_stop_command_to_robot(CALL, "ABORT", node.uid, g, ProcessingStatus::IDLE);

            *s = status::border; // listen neighbours, but not send messages
        }

        //! @brief Send a GOAL action to selected node and update the AP state machine of the robot to SELECTED
        FUN void send_action_to_selected_node(ARGS, process_tuple_type& p, goal_tuple_type const& g, status* s) {
            CODE
                std::string robot_chosen = get_real_robot_name(CALL, node.uid);
            std::cout << "Robot " << robot_chosen << " is chosen for goal " << common::get<goal_code>(g) << endl;
            std::cout << endl;

            // set processing status to SELECTED
            node.storage(node_process_status{}) = ProcessingStatus::SELECTED;

            // save goal
            node.storage(node_process_goal{}) = common::get<goal_code>(g);

            // send action to file
            action::ActionData action_data = {
                .action = common::get<goal_action>(g),
                .goal_code = common::get<goal_code>(g),
                .robot = robot_chosen,
                .pos_x = common::get<goal_pos_x>(g),
                .pos_y = common::get<goal_pos_y>(g),
                .pos_z = common::get<goal_pos_z>(g),
                .orient_w = common::get<goal_orient_w>(g)
            };
            action::manager::ActionManager::new_action(action_data);
        }

        // ACTION

        //! @brief Manage when the user has requested an ABORT of a goal
        FUN void manage_action_abort(ARGS, goal_tuple_type const& g, status* s) {
            CODE
                std::cout << "Process ABORT " << common::get<goal_code>(g) << " in node " << node.uid << " with status " << node.storage(node_process_status{}) << endl;
            // if and only if robot is in status RUNNING, sends stop command to robot
            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED) {
                send_stop_command_to_robot(CALL, "ABORT", node.uid, g, ProcessingStatus::TERMINATING);

                std::cout << "Robot " << node.uid << " is trying to terminate ABORT " << common::get<goal_code>(g) << endl;
            }
            else {
                if (AP_ENGINE_DEBUG) {
                    std::cout << "Robot " << node.uid << " is waiting to terminate ABORT " << common::get<goal_code>(g) << endl;
                }
            }
            *s = status::terminated_output; // stop propagation
        }

        //! @brief Manage when the user has requested a new GOAL
        FUN void manage_action_goal(ARGS, node_type nt, goal_tuple_type const& g, status* s, int n_round) {
            CODE
                if (AP_ENGINE_DEBUG) {
                }
            std::cout << "Process GOAL " << common::get<goal_code>(g) << ", action " << common::get<goal_action>(g) << " in node " << node.uid << endl;

            add_goal_to_computing_map(CALL, g);

            old(CALL, common::make_tagged_tuple<goal_code>(common::get<goal_code>(g)), [&](process_tuple_type p) {

                // compute charge of battery in percent
                float percent_charge = node.storage(node_battery_charge{}) / 100.0;

                // if i'm terminating the current goal, i have to terminate goal for all nodes
                if (common::get<goal_code>(g) == node.storage(node_process_goal{}) && //i was running current goal in the process
                    common::get<goal_code>(g) == node.storage(node_external_goal{}) && //the robot was running current goal
                    ProcessingStatus::TERMINATING == node.storage(node_process_status{})) { //but now i'm terminating
                    return ends_processed_goal(CALL, p, g, s);
                }

                // if battery is empty, then stop at current position
                if (node.storage(node_external_status{}) == feedback::GoalStatus::RUNNING && //i'm reaching the goal
                    percent_charge <= 0.0) { //the battery is full discharged
                    battery_discharged_when_it_is_running(CALL, p, g, s);
                }

                // TODO: add code from thesis


                // TODO: at the moment, AP sends command only to master
                if (nt == node_type::ROBOT_MASTER) {
                    int r = counter(CALL);
                    std::cout << "Conunter: " << r;
                    send_action_to_selected_node(CALL, p, g, s);
                }

                // blinking colors if not running
                if (common::get<goal_code>(g) != node.storage(node_process_goal{}) &&
                    node.storage(node_process_status{}) != ProcessingStatus::SELECTED) {
                    blink_computing_color(CALL, n_round);
                }

                // TODO
                return p;
                });
        }

        //! @brief Termination logic using share (see SHARE termination in ACSOS22 paper)
        FUN void termination_logic(ARGS, status& s, goal_tuple_type const&) {
            bool terminatingMyself = s == status::terminated_output;
            bool terminatingNeigh = nbr(CALL, terminatingMyself, [&](field<bool> nt) {
                return any_hood(CALL, nt) or terminatingMyself;
                });
            bool exiting = all_hood(CALL, nbr(CALL, terminatingNeigh), terminatingNeigh);
            if (exiting) s = status::border;
            else if (terminatingMyself) s = status::internal_output;
        }


        //! @brief Read new goals from shared variable and insert them in NewGoalsList
        // TODO: check performance of using std::transform 
        FUN void read_new_goals(ARGS, std::vector<goal_tuple_type>& NewGoalsList) {
            std::lock_guard lgg(GoalMutex);
            auto map_op = [](InputGoal ig) {
                return common::make_tagged_tuple<goal_action, goal_code, goal_pos_x, goal_pos_y, goal_pos_z, goal_orient_w, goal_source, goal_priority, goal_subcode>(
                    ig.action,
                    ig.goal_code,
                    ig.pos_x,
                    ig.pos_y,
                    ig.pos_z,
                    ig.orient_w,
                    ig.source,
                    ig.priority,
                    ig.subcode
                );
                };
            std::transform(InputGoalList.begin(), InputGoalList.end(), std::back_inserter(NewGoalsList), map_op);
            InputGoalList.clear();
        }

        // MAIN FUNCTIONS

        //! @brief Initialize MAIN function, selecting correct node_type
        FUN node_type init_main_fn(ARGS, int n_round) {
            node_type nt;

            if (AP_ENGINE_DEBUG) {
                std::cout << std::endl << std::endl;
                std::cout << "[node-" << node.uid << "] Time: " <<
                    std::chrono::time_point_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now()).time_since_epoch(
                        ).count() << endl;
            }

#if   defined(RUN_SIMULATION)
            if (node.uid == 0) {
                nt = node_type::ROBOT_MASTER;
            }
            else if (node.uid <= ROBOTS.size()) {
                nt = node_type::ROBOT_SLAVE;
            }
#elif defined(RUN_EMBEDDED)
            nt = node.uid == 0 ? node_type::ROBOT_MASTER : node_type::ROBOT_SLAVE;
#endif
            if (AP_ENGINE_DEBUG) {
                std::cout << "MAIN FUNCTION in node " << node.uid << " of type " << nt << endl;
            }

            field<int> test_nbr = fcpp::coordination::nbr(CALL, n_round);
            if (AP_ENGINE_DEBUG) {
                std::cout << "test_nbr_main_fn: " << test_nbr << "; nbr_uid: " << nbr_uid(CALL) << std::endl;
            }

            return nt;
        }


        //! @brief Acquire new goals from storage, in according with node_type.
        FUN void acquire_new_goals(ARGS, node_type nt, std::vector<goal_tuple_type>& NewGoalsList) {
#if FCPP_SYSTEM == FCPP_SYSTEM_GENERAL
            if (nt == node_type::ROBOT_MASTER) {
                // Guardare stato (running/selected) per decidere se prendere in carico goal 
                read_new_goals(CALL, NewGoalsList);
            }
#else
            if (nt == node_type::ROBOT_MASTER && EMBEDDED_NODE_KIOSK == node.uid) {
                read_new_goals(CALL, NewGoalsList);
            }
#endif
        }

        //! @brief Initialize variables (storage, etc...) of a robot using feedback data.
        FUN void init_robot(ARGS, std::string prefix) {
            // init settings
            if (node.storage(node_size{}) == 0) {
                node.storage(node_size{}) = NODE_SIZE;
            }
            if (node.storage(node_label_size{}) == 0) {
                node.storage(node_label_size{}) = LABEL_SIZE;
            }
            node.storage(node_label_text{}) = prefix + std::to_string(node.uid);
            node.storage(node_shape{}) = shape::sphere;
            node.storage(node_label_color{}) = fcpp::color(fcpp::BLACK);
            node.storage(node_shadow_color{}) = fcpp::color(0x837E7CFF);
            node.storage(node_shadow_shape{}) = shape::sphere;

            old(CALL, robot_phase::IDLE, [&](robot_phase ph) {
                std::string rname = ROBOT_PREFIX + std::to_string(node.uid); // name is obtained from node ID
                node.storage(node_external_name{}) = rname;

                std::lock_guard lgr(RobotStatesMutex);
                if (RobotStatesMap.find(rname) != RobotStatesMap.end()) {
                    for (RobotStatus rs : RobotStatesMap[rname]) {
                        // add offset to match with background
                        real_t pos_x_offset = node.storage(node_offset_pos_x{});
                        real_t pos_y_offset = node.storage(node_offset_pos_y{});
                        node.position() = make_vec(rs.pos_x + pos_x_offset, rs.pos_y + pos_y_offset, 0);

                        // truncate percentage to simply the output: 98 -> 90; 92 -> 90; 85 -> 80
                        float battery_percent_charge_trunc = std::trunc(rs.battery_percent_charge / 10) * 10;
                        if (rs.battery_percent_charge >= 0) {
                            // resize the node according with the battery percent charge
                            node.storage(node_size{}) = (battery_percent_charge_trunc / 100.0) * NODE_SIZE;
                            node.storage(node_shadow_size{}) = (battery_percent_charge_trunc / 100.0) * NODE_SHADOW_SIZE;
                        }
                        else {
                            node.storage(node_size{}) = NODE_SIZE / 2;
                            node.storage(node_shadow_size{}) = NODE_SHADOW_SIZE / 2;
                        }

                        // change colour according with feedback from robots
                        fcpp::color new_color = fcpp::color(fcpp::coordination::idle_color);
                        // if new state is RUNNING: change to "running color"
                        if (feedback::GoalStatus::RUNNING == rs.goal_status) {
                            manage_running_goal_status(CALL);
                        }
                        // if new state is REACHED:
                        else if (feedback::GoalStatus::REACHED == rs.goal_status) {
                            manage_reached_goal_status(CALL);
                        }
                        // if new state is UNKNOWN: new color is idle    
                        else if (feedback::GoalStatus::UNKNOWN == rs.goal_status) {
                            manage_unknown_goal_status(CALL);
                        }
                        // if new state is FAILED: new color is failed        
                        else if (feedback::GoalStatus::FAILED == rs.goal_status) {
                            manage_failed_goal_status(CALL);
                        }
                        // if new state is ABORTED: new color is aborted        
                        else if (feedback::GoalStatus::ABORTED == rs.goal_status) {
                            manage_aborted_goal_status(CALL);
                        }
                        // if new state is NO GOAL: new color is idle        
                        else if (feedback::GoalStatus::NO_GOAL == rs.goal_status) {
                            manage_no_goal_status(CALL);
                        }
                        // otherwise: maintain previous
                        else {
                            change_color(CALL, node.storage(node_color{}));
                        }

                        // update external status in storage
                        node.storage(node_external_status{}) = feedback::GoalStatus(rs.goal_status);
                        // update external goal in storage
                        node.storage(node_external_goal{}) = rs.goal_code;
                        node.storage(node_battery_charge{}) = battery_percent_charge_trunc;

                        // if battery is full discharged, change color to discharged
                        if (battery_percent_charge_trunc <= 0) {
                            manage_battery_discharged_node(CALL);
                        }
                    }
                    // delete element from map
                    RobotStatesMap.erase(rname);
                }

                return ph;
                });
        }

        // PROCESS MANAGEMENT

        //! @brief Spawn process from goal list acquired.
        FUN spawn_result_type spawn_process(ARGS, node_type nt, ::vector<goal_tuple_type>& NewGoalsList, int n_round) {
            // process new goals, emptying NewGoalsList
            return coordination::spawn(CALL, [&](goal_tuple_type const& g) {
                status s = status::internal_output;

                field<int> test_nbr = fcpp::coordination::nbr(CALL, n_round);
                if (AP_ENGINE_DEBUG) {
                    std::cout << "test_nbr_in_process: " << test_nbr << std::endl;
                }

                // ACTION: ABORT GOAL
                if (ABORT_ACTION == common::get<goal_action>(g) &&
                    node.storage(node_process_goal{}) == common::get<goal_code>(g) &&
                    node.storage(node_external_goal{}) == common::get<goal_code>(g)) {
                    manage_action_abort(CALL, g, &s);
                }

                // ACTION: REACH GOAL
                else if (GOAL_ACTION == common::get<goal_action>(g)) {
                    // If no others goals running
                    if (node.storage(node_external_status{}) != feedback::GoalStatus::RUNNING)
                        manage_action_goal(CALL, nt, g, &s, n_round);
                }

                termination_logic(CALL, s, g);
                return make_tuple(node.current_time(), s);
                }, NewGoalsList);
        }

        //! @brief Manage termination of the spawn processes.
        FUN void manage_termination(ARGS, node_type nt, spawn_result_type& r) {
            // if process was terminating and now it's terminated, we have to change state machine to IDLE 
            if (node.storage(node_process_status{}) == ProcessingStatus::TERMINATING) {
                // if process has been terminated, it isn't in the result map of spawn
                bool process_found = false;
                for (auto const& x : r)
                {
                    auto g = x.first;
                    if (common::get<goal_code>(g) == node.storage(node_process_goal{})) {
                        process_found = true;
                    }
                }
                if (!process_found) {
                    std::cout << "Process with code " << node.storage(node_process_goal{}) << " not found, so move robot to IDLE" << endl;
                    node.storage(node_process_status{}) = ProcessingStatus::IDLE;
                }
            }

            // search on results if the computing processes has been terminated: 
            //  - if it's terminated (or in other words, if the goal it's not in the "r" variable), we delete it from the map in the storage
            std::vector<std::string> goals_to_remove = {};
            for (auto const& x : node.storage(node_process_computing_goals{}))
            {
                auto g = x.second;
                if (r.find(g) == r.end()) {
                    std::cout << "Remove process with code " << common::get<goal_code>(g) << endl;
                    goals_to_remove.push_back(common::get<goal_code>(g));
                }
            }
            for (auto const& gc : goals_to_remove) {
                remove_goal_from_computing_map(CALL, gc);
            }
        }

        //! @brief Main case study function.
        MAIN()
        {
            // INITIALIZE VARS
            std::vector<goal_tuple_type> NewGoalsList{};
            int n_round = fcpp::coordination::counter(CALL);

            node_type nt = init_main_fn(CALL, n_round);

            // UPDATE DATA
            acquire_new_goals(CALL, nt, NewGoalsList);

            // INITIALIZE NODE AND STORAGE USING NEW DATA
            if (nt == node_type::ROBOT_MASTER)
            {
                init_robot(CALL, "RM.");
            }
            else if (nt == node_type::ROBOT_SLAVE)
            {
                init_robot(CALL, "RS.");
            }

            // PROCESS MANAGEMENT
            spawn_result_type r = spawn_process(CALL, nt, NewGoalsList, n_round);

            manage_termination(CALL, nt, r);

        }

        //! @brief Export types used by the *_connection functions.
        FUN_EXPORT any_connection_t = export_list<
            int,
            bool,
            goal_tuple_type,
            robot_phase,
            process_tuple_type
        >;

        //! @brief Export types used by the main function (update it when expanding the program).
        struct main_t : public export_list<
            any_connection_t,
            spawn_t<goal_tuple_type, status>,
            diameter_election_t<tuple<real_t, device_t>>
        > {};

    }
}

#endif // NODES_AP_ENGINE_H_
