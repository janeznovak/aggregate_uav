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
        FUN void change_color(ARGS, fcpp::color color) { CODE
            node.storage(left_color{})    = fcpp::color(color);
            node.storage(right_color{})   = fcpp::color(color);
            node.storage(node_color{})    = fcpp::color(color);
        }

        //! @brief Blink color of node
        FUN void blink_computing_color(ARGS, int n_round) { CODE
            fcpp::color computing_color = fcpp::color(fcpp::coordination::computing_colors[n_round%2]);
            change_color(CALL, computing_color);
        }

        //! @brief Get robot name from AP node_uid
        FUN string get_real_robot_name(ARGS, device_t node_uid) { CODE
            return get_robot_name(ROBOTS, static_cast<int>(node_uid));
        }

        //! @brief Update in the storage the tag "node_external_status_update_time"
        FUN void update_last_goal_update_time(ARGS) { CODE
            std::time_t now = std::time(nullptr);
            std::asctime(std::localtime(&now));
            node.storage(node_external_status_update_time{}) = now;
        }

        //! @brief Check if is passed "diff_time_ms" milliseconds from last REACHED change status
        FUN bool has_ms_passed_from_last_goal_update(ARGS, long diff_time_ms) { CODE
            std::time_t now = std::time(nullptr);
            std::chrono::milliseconds millis_to_check(diff_time_ms); 
            std::time_t stored_time = node.storage(node_external_status_update_time{});

            std::chrono::milliseconds millis_computed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::from_time_t(now) - std::chrono::system_clock::from_time_t(stored_time));
            return millis_computed >= millis_to_check;
        }

        //! @brief Add current goal to computing map
        FUN void add_goal_to_computing_map(ARGS, goal_tuple_type const& g) { CODE
            node.storage(node_process_computing_goals{})[common::get<goal_code>(g)] = g;
        }

        //! @brief Remove current goal to computing map
        FUN void remove_goal_from_computing_map(ARGS, std::string goal_code) { CODE
            node.storage(node_process_computing_goals{}).erase(goal_code);
        }

        //! @brief Check if exists other prioritised goals that are different from current
        FUN bool check_if_other_prioritised_goal_exists(ARGS, std::string current_goal_code) { CODE
            auto map = node.storage(node_process_computing_goals{});
            auto found = std::find_if(map.begin(), map.end(), [&](const auto& p){
                return 
                    common::get<goal_code>(p.second) != current_goal_code &&
                    common::get<goal_priority>(p.second) > 0;
            });
            if (found != map.end()) {
                return true;
            } else {
                // not found
                return false;
            }
        }

        //! @brief Check if the robot is enabled for specific subcode
        FUN bool check_if_im_enabled_for_subcode(ARGS, std::string subcode) { CODE
            return 
                (
                    node.storage(nodes_by_goal_subcode{}).find(subcode) == node.storage(nodes_by_goal_subcode{}).end() // not exists a configuration
                ) ||
                (
                    // exists a valid configuration
                    // and the current node it's in the list
                    std::find(
                        node.storage(nodes_by_goal_subcode{})[subcode].begin(), 
                        node.storage(nodes_by_goal_subcode{})[subcode].end(), 
                        node.uid
                    ) != node.storage(nodes_by_goal_subcode{})[subcode].end() 
                );
        }

        //! @brief Compute new rank for node, using battery and distance from goal
        FUN real_t run_rank_node(ARGS, float percent_charge, node_type nt, goal_tuple_type const& g) { CODE
            // compute distance to target
            real_t dist = norm(node.position() - make_vec(common::get<goal_pos_x>(g), common::get<goal_pos_y>(g), 0)); 
            real_t rank = (nt == node_type::ROBOT) 
                                ? dist * (1.0 - percent_charge )
                                : INF; 
            // rank setted to INF if:
            if (
                    // - node is failed for current goal and not already selected for something
                    (
                        node.storage(node_external_status{}) == feedback::GoalStatus::FAILED &&
                        node.storage(node_process_goal{}) == common::get<goal_code>(g) &&
                        node.storage(node_process_status{}) != ProcessingStatus::SELECTED
                    ) 
                        ||
                    // - or is already selected or running for another goal
                    (
                        (
                            node.storage(node_external_status{}) == feedback::GoalStatus::RUNNING ||
                            node.storage(node_process_status{}) == ProcessingStatus::SELECTED
                        ) &&
                        node.storage(node_process_goal{}) != common::get<goal_code>(g)
                    )
                        || 
                    // - or there is a subcode goal for specific group of nodes and i'm not enabled
                    (
                        (
                            common::get<goal_subcode>(g) != "" && // if the subcode is set
                            !check_if_im_enabled_for_subcode(CALL, common::get<goal_subcode>(g))
                        ) 
                    )
                        ||
                    // or if i'm processing not prioritised goal but there others prioritised goals
                    (
                        common::get<goal_priority>(g) == 0 &&
                        check_if_other_prioritised_goal_exists(CALL, common::get<goal_code>(g))
                    )
                        ||
                    // or it isn't passed necessary time from last completed goal (if configured)
                    (
                        ROBOT_PAUSE_SEC >= 0 && // if robot pause is enabled
                        node.storage(node_external_status{}) == feedback::GoalStatus::REACHED && // and the last external status is reached
                        !has_ms_passed_from_last_goal_update(CALL, ROBOT_PAUSE_SEC*1000) // and it's not passed configured time
                    )
                ) {
                rank = INF;
            }
            // rank setted to INF if percent charge of battery is 0.0
            else if (percent_charge == 0.0) {
               rank = INF; 
            }
            // rank setted to dist * 0.01 when battery is fully charged, otherwise the rank will be always 0:
            // the case when percent_charge is 1.0 (100%) becomes like percent_charge equals to 0.99 (99%)
            else if (percent_charge >= 1.0) {
                rank = dist * 0.01;
            }

            if (AP_ENGINE_DEBUG) {
                std::cout << "Computed local rank from node " << node.uid << " and for goal " << common::get<goal_code>(g) << " is: " << rank << endl;   
            }
            return rank;
        }

        //! @brief Send "stop" command to robot and change "node_process_status" after it
        FUN void send_stop_command_to_robot(ARGS, string action, device_t node_uid, goal_tuple_type const& g, ProcessingStatus ps) { CODE
            std::string robot_chosen = get_real_robot_name(CALL, node_uid);
            std::cout << "Robot "   << robot_chosen  << " is chosen for receiving ABORT command for goal " << common::get<goal_code>(g) << endl;
            std::cout << endl;

            action::ActionData action_data = {
                .action = action,
                .goal_code = common::get<goal_code>(g),
                .robot = robot_chosen,
                .pos_x = common::get<goal_pos_x>(g),
                .pos_y = common::get<goal_pos_y>(g),
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
        FUN void manage_battery_discharged_node(ARGS) { CODE
            change_color(CALL, fcpp::color(fcpp::coordination::discharged_color));
        }

        //! @brief Manage state machine when robot is running a goal
        FUN void manage_running_goal_status(ARGS) { CODE
            change_color(CALL, fcpp::color(fcpp::coordination::running_color));
        }

        //! @brief Manage state machine when robot has reached the goal
        FUN void manage_reached_goal_status(ARGS) { CODE
            fcpp::color new_color;
            // and previous state is NOT REACHED: new color is "reached" and deletes goal storage
            if (feedback::GoalStatus::REACHED != node.storage(node_external_status{})) {
                new_color = fcpp::color(fcpp::coordination::reached_goal_color);
                // resetting processing status
                node.storage(node_process_status{}) = ProcessingStatus::IDLE;
                // update time 
                update_last_goal_update_time(CALL);

            // and if process node is SELECTED, then AP has selected new node and simulation is ready to start    
            } else if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED) {
                new_color = fcpp::color(fcpp::coordination::running_color);
            
            // otherwise: new color is "reached"
            } else {
                new_color = fcpp::color(fcpp::coordination::reached_goal_color);
            }
            change_color(CALL, new_color);
        }

        //! @brief Manage state machine when robot reported an UNKNOWN error
        FUN void manage_unknown_goal_status(ARGS) { CODE
            fcpp::color new_color;
            // and if process node is SELECTED, then AP has selected new node and simulation is ready to start    
            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED) {
                new_color = fcpp::color(fcpp::coordination::running_color);
            } else {
                new_color = fcpp::color(fcpp::coordination::idle_color);
                // update time 
                update_last_goal_update_time(CALL);
            }
            change_color(CALL, new_color);
        }

        //! @brief Manage state machine when robot reported an abort of the current goal
        FUN void manage_aborted_goal_status(ARGS) { CODE
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
        FUN void manage_failed_goal_status(ARGS) { CODE
            fcpp::color new_color;
            // and previous state is not FAILED (first time of FAILED status): change internal status to IDLE
            if (feedback::GoalStatus::FAILED != node.storage(node_external_status{})) {
                new_color = fcpp::color(fcpp::coordination::failed_goal_color);
                // resetting processing status
                node.storage(node_process_status{}) = ProcessingStatus::IDLE;
                // update time  
                update_last_goal_update_time(CALL);

            // or previous state is FAILED (it's NOT the first time of FAILED status)
            } else {
                // and if process node is SELECTED, then AP has selected new node and simulation is ready to start    
                if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED) {
                    new_color = fcpp::color(fcpp::coordination::running_color);
                } else {
                    new_color = fcpp::color(fcpp::coordination::failed_goal_color);
                }
            }
            change_color(CALL, new_color);
        }

        //! @brief Manage state machine when robot has not yet executed a goal
        FUN void manage_no_goal_status(ARGS) { CODE
            fcpp::color new_color;
            // and if process node is SELECTED, then AP has selected new node and simulation is ready to start    
            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED) {
                new_color = fcpp::color(fcpp::coordination::running_color);
            } else {
                new_color = fcpp::color(fcpp::coordination::idle_color);
            }
            change_color(CALL, new_color);
        }

        // AP PROCESS

        //! @brief A robot has reached a goal and now try to terminate the process
        FUN rank_data_type ends_processed_goal(ARGS, 
            real_t prev_rank, int prev_leader_for_round, 
            device_t prev_lazy_detection_leader, int prev_lazy_detection_stable_for_round, 
            node_type nt, goal_tuple_type const& g, status* s) { CODE
                
                std::cout << "Robot "   << node.uid  << " is trying to terminate goal " << common::get<goal_code>(g) << endl;
                std::cout << endl;
                
                *s = status::terminated_output; // stop propagation
                return make_tuple(prev_rank, node.uid, prev_leader_for_round, make_tuple(prev_lazy_detection_leader, prev_lazy_detection_stable_for_round));
        }

        //! @brief A robot has discharged the battery, so AP send a stop command
        FUN void battery_discharged_when_it_is_running(ARGS, goal_tuple_type const& g, status* s) { CODE
            send_stop_command_to_robot(CALL, "ABORT", node.uid, g, ProcessingStatus::IDLE);

            *s = status::border; // listen neighbours, but not send messages
        }

        //! @brief Update the counter of how many times a robot is the leader
        // TODO: at the moment, the function re-use prev_leader_for_round to set the updated value.
        FUN void update_leader_counter(ARGS, float percent_charge, real_t prev_rank, device_t prev_leader, int& prev_leader_for_round, node_type nt, goal_tuple_type const& g, status* s) { CODE
            if (AP_ENGINE_DEBUG) {
                std::cout << "Using rank received from neighbours for " << common::get<goal_code>(g)  << ": (" << prev_rank << "; " << prev_leader << ")"  << endl;
            }
            if (nt == node_type::ROBOT && //i'm robot
                prev_rank != INF && //i have computed something
                percent_charge > 0.0 && // i have sufficient battery
                node.uid == prev_leader){ //i'm the best!

                // increment leader counter because i'm selected
                prev_leader_for_round++;
                std::cout << "Node with id " << node.uid << " selected " << common::get<goal_code>(g)  << " for " << prev_leader_for_round << " round " << endl;
            } else {
                // resetting leader counter because i'm not selected
                prev_leader_for_round = 0;
            }
        }

        //! @brief Send a GOAL action to selected node and update the AP state machine of the robot to SELECTED
        FUN void send_action_to_selected_node(ARGS, device_t prev_leader, goal_tuple_type const& g, status* s) { CODE
            std::string robot_chosen = get_real_robot_name(CALL, prev_leader); 
            std::cout << "Robot "   << robot_chosen  << " is chosen for goal " << common::get<goal_code>(g) << endl;
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
                .orient_w = common::get<goal_orient_w>(g)
            };
            action::manager::ActionManager::new_action(action_data);
        }

        //! @brief Detect if there are any other leaders near the elected robot.
        // If any exists, the worst node terminates his run.
        // TODO: attention because at the moment "prev_leader_for_round" it is resetted when another leader was found, so use it can be tricky
        FUN void detect_other_leaders(ARGS, tuple<real_t, device_t> new_rank_tuple, real_t rank, float percent_charge, real_t prev_rank, device_t prev_leader, int prev_leader_for_round, node_type nt, goal_tuple_type const& g, status* s) { CODE
            // get the leader of the next round
            real_t rank_new_leader      = get<0>(new_rank_tuple);
            device_t device_new_leader  = get<1>(new_rank_tuple);

            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED && //if i'm already selected
                node.storage(node_process_goal{}) == common::get<goal_code>(g) && // for the current goal
                node.storage(node_external_status{}) != feedback::GoalStatus::ABORTED) {//but i'm not aborted

                // search if new leader found will send an action in the next round
                if (device_new_leader != node.uid && // if i'm the old leader
                    rank_new_leader != INF) { // if new leader has a valid rank
                    std::cout << "Detected for goal " << common::get<goal_code>(g) << " other leader: "   << device_new_leader  << "(" << rank_new_leader << ",0) different from leader " << node.uid << "(" << rank <<", " << prev_leader_for_round << ")"<< endl;

                        // abort myself if i have worst rank: worst means "other"<"me"
                        if (rank_new_leader < rank) {
                            std::cout << "Abort current leader: "   << node.uid  << " from goal " << common::get<goal_code>(g) << endl;
                            send_stop_command_to_robot(CALL, "ABORT", node.uid, g, ProcessingStatus::IDLE);

                            *s = status::border; // listen neighbours, but not send messages
                        }
                } else {
                    if (AP_ENGINE_DEBUG) {
                        // std::cout << "Only one leader "   << " for goal " << common::get<goal_code>(g) << ": " << node.uid << endl;
                    }
                }
            } else {
                if (AP_ENGINE_DEBUG) {
                    // std::cout << "Slave heart beat: goal " << common::get<goal_code>(g) << " node leader "   << device_new_leader << endl;
                }
            }
        }
                                
        // ACTION

        //! @brief Manage when the user has requested an ABORT of a goal
        FUN void manage_action_abort(ARGS, goal_tuple_type const& g, status* s) { CODE
            std::cout << "Process ABORT "   << common::get<goal_code>(g) << " in node " << node.uid << " with status " << node.storage(node_process_status{}) << endl;
            // if and only if robot is in status RUNNING, sends stop command to robot
            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED) {
                send_stop_command_to_robot(CALL, "ABORT", node.uid, g, ProcessingStatus::TERMINATING);

                std::cout << "Robot "   << node.uid  << " is trying to terminate ABORT " << common::get<goal_code>(g) << endl;
            } else {
                if (AP_ENGINE_DEBUG) {
                    std::cout << "Robot "   << node.uid  << " is waiting to terminate ABORT " << common::get<goal_code>(g) << endl;
                }
            }
            *s = status::terminated_output; // stop propagation
        }

        //! @brief Manage when the user has requested a new GOAL
        FUN void manage_action_goal(ARGS, node_type nt, goal_tuple_type const& g, status* s, int n_round) { CODE
            if (AP_ENGINE_DEBUG) {
                std::cout << "Process GOAL "   << common::get<goal_code>(g)  << ", action " << common::get<goal_action>(g) << " in node " << node.uid  << endl;
            }

            add_goal_to_computing_map(CALL, g);
            // make_tuple(real_t, device_t, int, make_tuple(device_t, int)):
            // [0] -> rank of previous leader
            // [1] -> node uid of previous leader
            // [2] -> counter of how many rounds the leader is
            // [3] -> "lazy" leaders detection info
            //      [0] -> node uid of other leader detected
            //      [1] -> counter of how many rounds the detection info is stable
            old(CALL, make_tuple(INF, node.uid, 0, make_tuple(node.uid, 0)), [&] (rank_data_type prev_tuple) {

                // retrieve data from previous round
                real_t prev_rank = get<0>(prev_tuple);
                device_t prev_leader = get<1>(prev_tuple);
                int prev_leader_for_round = int(get<2>(prev_tuple));
                device_t prev_lazy_detection_leader = get<0>(get<3>(prev_tuple));
                int prev_lazy_detection_stable_for_round = int(get<1>(get<3>(prev_tuple)));

                // compute charge of battery in percent
                float percent_charge = node.storage(node_battery_charge{})/100.0;
                
                if (nt == node_type::ROBOT) {//i'm robot
                    // if i reached current goal, then terminate the process
                    if (common::get<goal_code>(g) == node.storage(node_process_goal{}) && //i was running current goal in the process
                        common::get<goal_code>(g) == node.storage(node_external_goal{}) && //the robot was running current goal
                        feedback::GoalStatus::REACHED == node.storage(node_external_status{}) && //the robot reached the goal
                        ProcessingStatus::SELECTED != node.storage(node_process_status{})) { //but now i'm idle
                        return ends_processed_goal(CALL, prev_rank, prev_leader_for_round, prev_lazy_detection_leader, prev_lazy_detection_stable_for_round, nt, g, s);
                    } 

                    // if i'm terminating the current goal, i have to terminate goal for all nodes
                    if (common::get<goal_code>(g) == node.storage(node_process_goal{}) && //i was running current goal in the process
                        common::get<goal_code>(g) == node.storage(node_external_goal{}) && //the robot was running current goal
                        ProcessingStatus::TERMINATING == node.storage(node_process_status{})) { //but now i'm terminating
                        return ends_processed_goal(CALL, prev_rank, prev_leader_for_round, prev_lazy_detection_leader, prev_lazy_detection_stable_for_round, nt, g, s);
                    } 

                    // if battery is empty, then stop at current position
                    if (node.storage(node_external_status{}) == feedback::GoalStatus::RUNNING && //i'm reaching the goal
                        percent_charge <= 0.0) { //the battery is full discharged
                        battery_discharged_when_it_is_running(CALL, g, s);
                    }
                }

                update_leader_counter(CALL, percent_charge, prev_rank, prev_leader, prev_leader_for_round, nt, g, s);
                
                // compute rank for this node
                real_t rank = INF;
                if (nt != node_type::WEARABLE) {
                    rank = run_rank_node(CALL, percent_charge, nt, g);   
                }

                real_t other_leader_rank = INF;
                device_t other_leader_device = node.uid;
                if (ALG_USED == ElectionAlgorithm::LAZY) {
                    // get other leaders from all known network 
                    bool im_running_goal = (
                            (
                                node.storage(node_process_status{}) == ProcessingStatus::SELECTED || //if i'm already selected
                                node.storage(node_process_status{}) == ProcessingStatus::TERMINATING //or i'm terminating
                            ) 
                        &&
                            node.storage(node_process_goal{}) == common::get<goal_code>(g) && // for the current goal
                            node.storage(node_external_status{}) != feedback::GoalStatus::ABORTED // and different status from aborted
                    );
                    // use diameter_election to check other leaders
                    tuple<real_t, device_t> tuple_leaders = fcpp::coordination::diameter_election(
                        CALL, 
                        mux(
                            im_running_goal, 
                            make_tuple(rank, node.uid),
                            make_tuple(INF, node.uid)
                        ),
                        fcpp::coordination::graph_diameter
                    );
                    other_leader_rank = get<0>(tuple_leaders);
                    other_leader_device = get<1>(tuple_leaders);
                }

                if (nt == node_type::ROBOT && //i'm robot
                    prev_rank != INF && //i have computed something previously
                    rank != INF && //and also now i have computed something
                    node.storage(node_process_status{}) == ProcessingStatus::IDLE && //i'm IDLE, so ready to go!
                    percent_charge > 0.0 && //i have sufficient battery
                    node.uid == prev_leader && //i'm the best!
                    prev_leader_for_round >= fcpp::coordination::graph_diameter){ //selected as leader for sufficient rounds

                    if (ALG_USED == ElectionAlgorithm::LAZY) {
                        if (other_leader_device == prev_lazy_detection_leader) { // if it's equals to previous leader
                            // increment the counter
                            prev_lazy_detection_stable_for_round++;
                        }else {
                            // otherwise reset the counter
                            prev_lazy_detection_stable_for_round = 0;
                        }
                        
                        // use information about "other leaders" only if the value of "other_leader_rank" is stable:
                        // we are conservative, because the value can be "not aligned"
                        if (prev_lazy_detection_stable_for_round >= fcpp::coordination::graph_diameter) { //the other leader is "stable" for N round
                            if (other_leader_rank != INF) { // other leader computes valid rank 
                                std::cout << "Block action goal for node " << node.uid << " with rank " << prev_rank << " for goal " << common::get<goal_code>(g) << " because there is another leader for consecutively " << prev_lazy_detection_stable_for_round << " round: (" << other_leader_device  << ", " << other_leader_rank << ")" << endl;
                            } else {
                                send_action_to_selected_node(CALL, prev_leader, g, s);
                            }
                        } else {
                            std::cout << "Skip lazy leader detection for node " << node.uid << " because it's not stable yet" << endl;  
                        }
                        
                        // update previous other leader
                        prev_lazy_detection_leader = other_leader_device;
                    } else {
                        // if LAZY mode is not enabled, send action immediately
                        send_action_to_selected_node(CALL, prev_leader, g, s);
                    }
                }

                // blinking colors if not running
                if (common::get<goal_code>(g) != node.storage(node_process_goal{}) && 
                    node.storage(node_process_status{}) != ProcessingStatus::SELECTED) {
                    blink_computing_color(CALL, n_round);
                }
                
                // send to neighbours result of diameter election and retrieve the best node for next round
                tuple<real_t, device_t> new_rank_tuple = fcpp::coordination::diameter_election(
                    CALL, 
                    make_tuple(rank, node.uid), 
                    fcpp::coordination::graph_diameter
                );

                if (AP_ENGINE_DEBUG) {
                    std::cout << "New rank found by node " << node.uid << " for " << common::get<goal_code>(g) << ": " << new_rank_tuple << std::endl;
                }

                if (nt == node_type::ROBOT && //i'm robot
                    ALG_USED == ElectionAlgorithm::GREEDY) { 
                    //detect if there will be other leaders
                    detect_other_leaders(CALL, new_rank_tuple, rank, percent_charge, prev_rank, prev_leader, prev_leader_for_round, nt, g, s);
                }  
                return make_tuple(  get<0>(new_rank_tuple), 
                                    get<1>(new_rank_tuple), 
                                    prev_leader_for_round,
                                    make_tuple(
                                        prev_lazy_detection_leader,
                                        prev_lazy_detection_stable_for_round
                                    )
                        );
            });
        }

        //! @brief Termination logic using share (see SHARE termination in ACSOS22 paper)
        FUN void termination_logic(ARGS, status& s, goal_tuple_type const&) {
            bool terminatingMyself = s == status::terminated_output;
            bool terminatingNeigh = nbr(CALL, terminatingMyself, [&](field<bool> nt){
                return any_hood(CALL, nt) or terminatingMyself;
            });
            bool exiting = all_hood(CALL, nbr(CALL, terminatingNeigh), terminatingNeigh);
            if (exiting) s = status::border; 
            else if (terminatingMyself) s = status::internal_output;
        }

        //! @brief Read new goals from shared variable and insert them in NewGoalsList
        // TODO: check performance of using std::transform 
        FUN void read_new_goals(ARGS, std::vector<goal_tuple_type>& NewGoalsList, string source) {
            std::vector<InputGoal> InputGoalsBySource;
            std::copy_if(InputGoalList.begin(), InputGoalList.end(), std::back_inserter(InputGoalsBySource), [&](InputGoal ig) {
                return ig.source == source;
            });
            
            std::lock_guard lgg(GoalMutex);
            auto map_op = [](InputGoal ig) {
                return common::make_tagged_tuple<goal_action, goal_code, goal_pos_x, goal_pos_y, goal_orient_w, goal_source, goal_priority, goal_subcode>(
                    ig.action,
                    ig.goal_code,
                    ig.pos_x,
                    ig.pos_y,
                    ig.orient_w,
                    ig.source,
                    ig.priority,
                    ig.subcode
                );
            };
            std::transform(InputGoalsBySource.begin(), InputGoalsBySource.end(), std::back_inserter(NewGoalsList), map_op);

            // delete only goals with source = node_external_name
            InputGoalList.erase(
                std::remove_if(InputGoalList.begin(), InputGoalList.end(), [&](InputGoal ig) {
                    return ig.source == source;
                }),
                InputGoalList.end()
            );
        }

        //! @brief Read new goals from shared variable and insert them in NewGoalsList
        // TODO: check performance of using std::transform 
        FUN void read_new_goals(ARGS, std::vector<goal_tuple_type>& NewGoalsList) {
            std::lock_guard lgg(GoalMutex);
            auto map_op = [](InputGoal ig) {
                return common::make_tagged_tuple<goal_action, goal_code, goal_pos_x, goal_pos_y, goal_orient_w, goal_source, goal_priority, goal_subcode>(
                    ig.action,
                    ig.goal_code,
                    ig.pos_x,
                    ig.pos_y,
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
                    nt = node_type::KIOSK;
                } else if (node.uid <= ROBOTS.size()) {
                    nt = node_type::ROBOT;
                } else if (node.uid <= ROBOTS.size() + WEARABLES.size()) {
                    nt = node_type::WEARABLE;
                }
            #elif defined(RUN_EMBEDDED)
                nt = node.uid == 0 ? node_type::KIOSK : node_type::ROBOT;
            #endif
            if (AP_ENGINE_DEBUG) {
                std::cout << "MAIN FUNCTION in node " << node.uid << " of type " << nt << endl;
            }

            field<int> test_nbr = fcpp::coordination::nbr(CALL, n_round);
            if (AP_ENGINE_DEBUG) {
                std::cout << "test_nbr_main_fn: " << test_nbr  << "; nbr_uid: " << nbr_uid(CALL) << std::endl;
            }

            return nt;
        }


        //! @brief Acquire new goals from storage, in according with node_type.
        FUN void acquire_new_goals(ARGS, node_type nt, std::vector<goal_tuple_type>& NewGoalsList) {
            #if FCPP_SYSTEM == FCPP_SYSTEM_GENERAL
                if (WEARABLES.size() > 0) {
                    if (nt == node_type::WEARABLE) {
                        // keep only goals with source = node_external_name
                        read_new_goals(CALL, NewGoalsList, node.storage(node_external_name{}));
                    }
                    if (nt == node_type::KIOSK) {
                        // kiosk read only goals with source = ""
                        read_new_goals(CALL, NewGoalsList, "");
                    }
                } else {
                    if (nt == node_type::KIOSK) {
                        read_new_goals(CALL, NewGoalsList);
                    }
                }
            #else
                if (nt == node_type::ROBOT && EMBEDDED_NODE_KIOSK == node.uid) {
                    read_new_goals(CALL, NewGoalsList);
                }
            #endif
        }

        //! @brief Initialize variables (storage, etc...) of a kiosk.
        FUN void init_kiosk(ARGS) {
            // init settings: move to 0.0.0
            node.position() = make_vec(0,0,0); 
            node.storage(node_shape{}) = shape::cube;
            change_color(CALL, fcpp::color(RED));
            node.storage(node_size{})  = 0.1;
        }

        //! @brief Initialize variables (storage, etc...) of a robot using feedback data.
        FUN void init_robot(ARGS) {
            // init settings
            if (node.storage(node_size{}) == 0) {
                node.storage(node_size{}) = NODE_SIZE;
            }
            if (node.storage(node_label_size{}) == 0) {
                node.storage(node_label_size{}) = LABEL_SIZE;
            }
            node.storage(node_label_text{})   = "R."+std::to_string(node.uid);
            node.storage(node_shape{})        = shape::sphere;
            node.storage(node_label_color{})  = fcpp::color(fcpp::BLACK);
            node.storage(node_shadow_color{}) = fcpp::color(0x837E7CFF);
            node.storage(node_shadow_shape{}) = shape::sphere;

            old(CALL, robot_phase::IDLE, [&](robot_phase ph){
                std::string rname = ROBOT_PREFIX + std::to_string(node.uid); // name is obtained from node ID
                node.storage(node_external_name{}) = rname;

                std::lock_guard lgr(RobotStatesMutex);
                if (RobotStatesMap.find(rname) != RobotStatesMap.end()) {
                    for(RobotStatus rs : RobotStatesMap[rname]) {
                        // add offset to match with background
                        real_t pos_x_offset = node.storage(node_offset_pos_x{});
                        real_t pos_y_offset = node.storage(node_offset_pos_y{});
                        node.position() = make_vec(rs.pos_x+pos_x_offset,rs.pos_y+pos_y_offset,0);

                        // truncate percentage to simply the output: 98 -> 90; 92 -> 90; 85 -> 80
                        float battery_percent_charge_trunc = std::trunc(rs.battery_percent_charge/10)*10;
                        if (rs.battery_percent_charge >= 0) {
                            // resize the node according with the battery percent charge
                            node.storage(node_size{}) = (battery_percent_charge_trunc/100.0)*NODE_SIZE;
                            node.storage(node_shadow_size{})  = (battery_percent_charge_trunc/100.0)*NODE_SHADOW_SIZE;
                        } else {
                            node.storage(node_size{}) = NODE_SIZE/2;
                            node.storage(node_shadow_size{})  = NODE_SHADOW_SIZE/2;
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
                            change_color(CALL,node.storage(node_color{}));
                        }

                        // update external status in storage
                        node.storage(node_external_status{})  = feedback::GoalStatus(rs.goal_status);
                        // update external goal in storage
                        node.storage(node_external_goal{})    = rs.goal_code;
                        node.storage(node_battery_charge{})   = battery_percent_charge_trunc;

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

        //! @brief Initialize variables (storage, etc...) of a wearable using feedback data.
        FUN void init_wearable(ARGS) {
            // init settings
            if (node.storage(node_size{}) == 0) {
                node.storage(node_size{}) = NODE_SIZE;
            }
            if (node.storage(node_label_size{}) == 0) {
                node.storage(node_label_size{}) = LABEL_SIZE;
            }
            node.storage(node_shape{})        = shape::star;
            node.storage(node_label_color{})  = fcpp::color(fcpp::BLACK);
            node.storage(node_shadow_color{}) = fcpp::color(0x837E7CFF);
            node.storage(node_shadow_shape{}) = shape::star;

            old(CALL, robot_phase::IDLE, [&](robot_phase ph){
                std::string wearable_id = std::to_string(node.uid-ROBOTS.size());
                std::string wname = WEARABLE_PREFIX + wearable_id; 
                node.storage(node_external_name{}) = wname;
                node.storage(node_label_text{}) = "P." + wearable_id;

                std::lock_guard lgr(WearableStatesMutex);
                if (WearableStatesMap.find(wname) != WearableStatesMap.end()) {
                    for(WearableStatus ws : WearableStatesMap[wname]) {
                        node.position() = make_vec(ws.pos_x,ws.pos_y,0);

                        // change colour according with feedback from wearable
                        fcpp::color new_color = fcpp::color(fcpp::coordination::idle_color);

                        // if new state is RUNNING: change to "running color"
                        if (wearable_feedback::GoalStatus::RUNNING == ws.goal_status) {
                            new_color = fcpp::color(fcpp::coordination::running_color);
                        } else if (wearable_feedback::GoalStatus::REACHED == ws.goal_status) {
                            new_color = fcpp::color(fcpp::coordination::reached_goal_color);
                        }
                        change_color(CALL,new_color);
                    }
                    // delete element from map
                    WearableStatesMap.erase(wname);
                }

                return ph;
            });
        }

        // PROCESS MANAGEMENT

        //! @brief Spawn process from goal list acquired.
        FUN spawn_result_type spawn_process(ARGS, node_type nt, ::vector<goal_tuple_type>& NewGoalsList, int n_round) {
            // process new goals, emptying NewGoalsList
            return coordination::spawn(CALL, [&](goal_tuple_type const& g){
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
                else if (GOAL_ACTION == common::get<goal_action>(g)){
                    manage_action_goal(CALL, nt, g, &s, n_round);
                }

                // ACTION: SOS GOAL
                else if (SOS_ACTION == common::get<goal_action>(g)){
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
                    if (common::get<goal_code>(g) == node.storage(node_process_goal{})){
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
                if (r.find(g) == r.end()){
                    std::cout << "Remove process with code " << common::get<goal_code>(g) << endl;
                    goals_to_remove.push_back(common::get<goal_code>(g));
                } 
            }
            for (auto const& gc : goals_to_remove) {
                remove_goal_from_computing_map(CALL, gc);

                // return in IDLE color after removing computing goal in wearable node
                if (nt == node_type::WEARABLE) {
                    change_color(CALL, fcpp::color(fcpp::coordination::idle_color));
                }
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
            if (nt == node_type::KIOSK)
            {
                init_kiosk(CALL);
            }
            else if (nt == node_type::ROBOT)
            {
                init_robot(CALL);
            }
            else if (nt == node_type::WEARABLE)
            {
                init_wearable(CALL);
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
            rank_data_type
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
