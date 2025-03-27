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
#include <fstream>
#include <iostream>

#include "lib/poc_config.hpp"
#include "lib/fcpp.hpp"
#include "lib/file_utils.hpp"

#include "lib/common_data.hpp"
#include "lib/ap_engine_setup.hpp"
#include "lib/action_writer.hpp"
#include <cstdlib>

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

        FUN string get_robot_id_from_goal_code(ARGS, string goal_code) {
            CODE
                std::vector<std::string> tokens;
                size_t start = 0;
                size_t end = goal_code.find("-");
                while (end != std::string::npos) {
                    tokens.push_back(goal_code.substr(start, end - start));
                    start = end + 1;
                    end = goal_code.find("-", start);
                }
                tokens.push_back(goal_code.substr(start, end));
                
                return tokens[2];
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

        // Flocking Utils



        bool compare(double a, double b) {
            return a < b;
        }

        void log_error(string uid, string time, double error){
            std::string out_path = string(OUTPUT_FOLDER_BASE_PATH) + string("from_ap/to_log/");
            create_folder_if_not_exists(out_path);

            std::ofstream file;
            std::string uniqueFilename = out_path + "/" + uid+"_error_log.txt";

            file.open(uniqueFilename, std::ios::app);

            if (file.is_open()) {
                file << time << "," << std::to_string(error) << std::endl;
                file.close();
            }
            else {
                std::cout << "Error log file." << std::endl;
            }
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

        // FLOCKING

        //! @brief controllo l'indice e nel caso non fosse valido lo riassegno...
        FUN void checkIndex(ARGS, node_type nt) {
            using namespace tags;
            if (nt == node_type::ROBOT_SLAVE) {
                if ((get<1>(node.storage(node_indexSlave{}))) > 0) {
                    field <tuple<int, int>> f = nbr(CALL, node.storage(node_indexSlave{}));
                    tuple<int, int> check = max_hood(CALL, f);
                    if (get<1>(node.storage(node_indexSlave{})) == get<1>(check) && get<0>(node.storage(node_indexSlave{})) != get<0>(check)) {
                        node.storage(node_checkIndex{}) = true;
                        if (!(node.storage(node_secondReturn{}))) {
                            get<1>(node.storage(node_indexSlave{})) += 1;
                        }
                        else {
                            get<1>(node.storage(node_indexSlave{})) -= 1;
                        }
                    }
                }
            }
        }


        // to calculate where in the circumference the slave is
        FUN void calculateMyCorner(ARGS) {
            using namespace tags;
            // if it has a master
            if (get<0>(node.storage(node_posMaster{}))) {
                double myRadiant = (get<1>(node.storage(node_indexSlave{}))) * ((2 * pi) / node.storage(node_numberOfSlave{}));
                node.storage(node_myRadiant{}) = myRadiant;
                double sine_value = std::sin(myRadiant);
                double cosine_value = std::cos(myRadiant);
                double x = sine_value * distanceMasterSlave;
                double y = cosine_value * distanceMasterSlave;
                if (node.storage(node_checkIndex{})) {
                    x = sine_value * distanceCircularCrown;
                    y = cosine_value * distanceCircularCrown;
                }
                vec<3> vecMyRadiant = make_vec(x, y, 0); 
                node.storage(node_vecMyRadiant{}) = vecMyRadiant;

                /**traslazione del cerchio attorno al master*/
                vec<3> dist = get<1>(node.storage(node_posMaster{})) - node.position();
                vec<3> versore = vecMyRadiant + dist;
                node.storage(node_vecMyVersor{}) = versore;
                // if (!(isnan(versore[0]) && isnan(versore[1]))) {
                    // node.propulsion() = versore / norm(versore);
                // }
                // node.propulsion() += node.propulsion() * incrementAcceleration;
            }
        }

        //! @brief se dei nodi slave escono dalla circonferenza "resetto" gli indici degli slave all'interno della circonferenza che superano il massimo indice...
        FUN bool decrementIndex(ARGS) {
            using namespace tags;
            int difference = node.storage(node_maxNumberOfSlave{}) - node.storage(node_numberOfSlave{});
            if (difference > 0 && get<1>(node.storage(node_indexSlave{})) > node.storage(node_numberOfSlave{})) {
                get<1>(node.storage(node_indexSlave{})) = 1;
                return false;
            }
            return true;
        }


        FUN void errorCalculator(ARGS) {
            using namespace tags;
            if (get<0>(node.storage(node_posMaster{}))) {
                double theta = (get<1>(node.storage(node_indexSlave{}))) * ((2 * pi) / node.storage(node_numberOfSlave{}));
                double x = std::sin(theta) * distanceMasterSlave;
                double y = std::cos(theta) * distanceMasterSlave;
                vec<3> vecTheta = make_vec(x, y, 0);
                vec<3> exactPosition = vecTheta + get<1>(node.storage(node_posMaster{}));
                double error = distance(exactPosition, node.position());
                double normalizedError = error / distanceMasterSlave;  // Normalizzazione dell'errore
                node.storage(position_error{}) = error;
                log_error(
                    std::to_string(node.uid), 
                    std::to_string(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count()), 
                    error);
            }
        }

        FUN void collisionAvoidance(ARGS, node_type nt) {
            using namespace tags;
            if (nt == node_type::ROBOT_SLAVE) {
                if (get<0>(node.storage(node_posMaster{}))) {
                    bool flag = compare(distance(get<1>(node.storage(node_posMaster{})), node.position()), minDistance);
                    vec<3> elasticMaster = make_vec(0, 0, 0);
                    vec<3> v = get<1>(node.storage(node_posMaster{})) - node.position();
                    if (!node.storage(node_checkIndex{})) {
                        elasticMaster = v * (1 - distanceMasterSlave / (norm(v))) * hardnessMasterSlave;
                    }
                    else {
                        elasticMaster = v * ((1 - distanceCircularCrown / (norm(v)))) * hardnessCircularCrown;
                    }
                    if (flag) {
                        //! Viene considerata come limite critico di livello 2 la distanza minima consentita
                        elasticMaster += elasticMaster * incrementForce;
                    }
                    node.storage(node_collisionAvoidanceMaster{}) = elasticMaster;

                    //! Viene considerata come limite critico di livello 1 il 90% della suddivisione in parti uguali per permettere
                    //! un margine d'errore del 10%
                    double distanceSlaveSlave = (2 * distanceMasterSlave * pi) / node.storage(node_numberOfSlave{});

                    tuple<bool, vec<3>> elasticSlave = sum_hood(CALL, map_hood([](vec<3> v, double d, double l, double constAvoid) {
                        tuple<bool, vec<3>> t = make_tuple(false, make_vec(0, 0, 0));
                        double criticalLimit = l * 0.9;
                        if (d < criticalLimit) {
                            get<1>(t) = v * ((1 - l / (norm(v))) * hardnessSlaveSlave);
                            //! Viene considerata come limite critico di livello 2 la distanza minima consentita
                            if (d < constAvoid) {
                                get<1>(t) += get<1>(t) * incrementForce;
                            }
                        }
                        return t;
                        }, node.nbr_vec(), node.nbr_dist(), distanceSlaveSlave, minDistance), tuple<bool, vec<3>>{});
                    node.storage(node_collisionAvoidanceSlaves{}) = get<1>(elasticSlave);
                    if (node.storage(node_collisionAvoidanceSlaves{}) != make_vec(0, 0, 0)) {
                        node.storage(node_flagDistance{}) = true;
                    }
                    else {
                        node.storage(node_flagDistance{}) = false;
                    }

                    //! Viene semplicemente controllato che i valori calcolati siano assegnabili, ovvero !NaN || !inf
                    if ((!(isnan(node.storage(node_collisionAvoidanceMaster{})[0])) && !(std::isinf(node.storage(node_collisionAvoidanceMaster{})[0]))) ||
                        (!(isnan(node.storage(node_collisionAvoidanceMaster{})[1])) && !(std::isinf(node.storage(node_collisionAvoidanceMaster{})[1])))) {

                        node.storage(node_vecMyVersor{}) += node.storage(node_collisionAvoidanceMaster{});
                        if ((!(isnan(node.storage(node_collisionAvoidanceSlaves{})[0])) && !(std::isinf(node.storage(node_collisionAvoidanceSlaves{})[0]))) ||
                            (!(isnan(node.storage(node_collisionAvoidanceSlaves{})[1])) && !(std::isinf(node.storage(node_collisionAvoidanceSlaves{})[1])))) {

                            node.storage(node_vecMyVersor{}) += node.storage(node_collisionAvoidanceSlaves{});
                        }
                    }
                }
            }
        }

        FUN void initialization(ARGS, node_type nt) {
            using namespace tags;
            int slaves = 0;

            // tuple of is_master and position
            tuple<bool, vec<3>> t = make_tuple(false, make_vec(0, 0, 0));
            if (nt == node_type::ROBOT_MASTER) {
                t = make_tuple(true, node.position());
            }

            float percent_charge = node.storage(node_battery_charge{}) / 100.0;
            if(percent_charge > 0){
                slaves = (int)count_hood(CALL) - 1;
                // slave doesn't have slaves
                if (nt == node_type::ROBOT_SLAVE) {
                    slaves = 0;
                }
            }

            // number of slaves, you propagate the number of slaves and save the maximum number of slaves
            field<int> identifierNumSlaves = nbr(CALL, 0, slaves);
            node.storage(node_numberOfSlave{}) = max_hood(CALL, identifierNumSlaves);

            field < tuple < bool, vec <3>>> identifierMaster = nbr(CALL, t);
            // try to find the master(slaves have 0(false) for bool and master has something bigger(true))
            tuple<bool, vec<3>> identified = max_hood(CALL, identifierMaster);
            // go in the block of code if the node has identified the master, even if your master dies, you can get a new one with propagation
            if (get<0>(identified)) {
                node.storage(node_posMaster{}) = identified;
                /**Quando un nodo identifica la posizione del master gli viene assegnato un indice che equivale
                 * all'ordine di arrivo all'interno del cerchio...(servirà nella funzione myCorner)*/
                if (!(node.storage(node_secondReturn{}))) {
                    int maxIndex = nbr(CALL, 0, [&](field<int> indexes) {
                        int maxIndex = max_hood(CALL, indexes);
                        // also checks if the robot is a slave, so that you don't mind the index of the master
                        if (get<1>(node.storage(node_indexSlave{})) == 0 && nt == node_type::ROBOT_SLAVE) {
                            return maxIndex + 1;
                        }
                        else {
                            return maxIndex;
                        }
                        });

                    if ((get<1>(node.storage(node_indexSlave{}))) == 0 && nt == node_type::ROBOT_SLAVE) {
                        node.storage(node_indexSlave{}) = make_tuple(node.uid, maxIndex);
                    }
                }
                else {
                    if ((get<1>(node.storage(node_indexSlave{}))) == 0 && nt == node_type::ROBOT_SLAVE) {
                        // set the index just to the amount of slaves there is
                        get<1>(node.storage(node_indexSlave{})) = node.storage(node_numberOfSlave{});
                    }
                }
            }
            else{
                // I think here the node_posMaster{} gets a(some) slave TODO: check this 
                node.storage(node_posMaster{}) = identified;
            }

            // propagete the maximum number of slaves, if current is bigger, than save it, otherwise take b
            node.storage(node_maxNumberOfSlave{}) = old(CALL, 0, [&](int b) {
                return max(b, (node.storage(node_numberOfSlave{})));
            });
        }


        FUN void run_flocking(ARGS, node_type nt) {
            CODE
                /**controllo nel caso degli indici si sovrapponessero*/
                field <tuple<int, int>> identifierWrongIndex = nbr(CALL, node.storage(node_indexSlave{}));

            // Aggiungere a true la batteria è a 0 
            bool flagIndex = all_hood(CALL, map_hood([](tuple<int, int> t, tuple<int, int> myValue) {
                if (get<1>(myValue) == get<1>(t) && get<0>(myValue) != get<0>(t)) {
                    return false;
                }
                else {
                    return true;
                }
                }, identifierWrongIndex, node.storage(node_indexSlave{})));

            if (!(flagIndex && decrementIndex(CALL))) {
                checkIndex(CALL, nt);
            }
            else {
                node.storage(node_checkIndex{}) = false;
            }

            if (nt == node_type::ROBOT_SLAVE) {
                // instead calculateMyCorner we should have the slave move to a
                // position of interest based on something(maybe master, maybe goal from
                // a user, ...)
                calculateMyCorner(CALL);
                //!Sistema di collision avoidance
                collisionAvoidance(CALL, nt);
                errorCalculator(CALL);
            }

            if (nt == node_type::ROBOT_SLAVE) {
                if (!(get<0>(node.storage(node_posMaster{})))) {
                    node.storage(node_startPosition{}) = node.position();
                }
                else {
                    node.storage(node_startPosition{}) = node.position();
                }
            }
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
            *s = status::border; // listen neighbours, but not send messages
        }

        //! @brief Send a GOAL action to selected node and update the AP state machine of the robot to SELECTED
        FUN void send_action_to_selected_node(ARGS, node_type nt, process_tuple_type& p, goal_tuple_type const& g, status* s) {
            CODE
                std::string robot_chosen = get_real_robot_name(CALL, node.uid);
                std::string goal_code_robot = get_real_robot_name(CALL, std::stoi(get_robot_id_from_goal_code(CALL, common::get<goal_code>(g))));
                int goal_code_robot_id = std::stoi(get_robot_id_from_goal_code(CALL, common::get<goal_code>(g)));

            if (nt == node_type::ROBOT_MASTER && node.uid == goal_code_robot_id) {
                // here I think we should change the code so that it's not reading
                // the goal directly from a file with a predetermined path, but
                // instead it should get the goal from the slaves which exemined
                // where the goal is
                // TODO: check if the goal is the one from the file or is it from
                // creating a new goal
                std::cout << "Robot " << robot_chosen << " is chosen for goal " << common::get<goal_code>(g) << endl;
                // print the goal_code_robot_id and node.uid
                std::cout << "Goal code robot id: " << goal_code_robot_id << " Node uid: " << node.uid << endl;
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
                // print the position of x
                std::cout << "Position x: " << common::get<goal_pos_x>(g) << endl;
            }
            else{
                // send action to file a.k.a send the goal to the scout/slave
                action::ActionData action_data = {
                    .action = common::get<goal_action>(g),
                    .goal_code = common::get<goal_code>(g),
                    .robot = robot_chosen,
                    .pos_x = (float)node.storage(node_vecMyVersor{})[0],
                    .pos_y = (float)node.storage(node_vecMyVersor{})[1],
                    .pos_z = (float)node.storage(node_vecMyVersor{})[2],
                    .orient_w = 0.0
                };
                action::manager::ActionManager::new_action(action_data);
            }
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

                std::cout << "Process GOAL " << common::get<goal_code>(g) << ", action " << common::get<goal_action>(g) << " in node " << node.uid << endl;

            add_goal_to_computing_map(CALL, g);

            old(CALL, common::make_tagged_tuple<goal_code>(common::get<goal_code>(g)), [&](process_tuple_type p) {

                // compute charge of battery in percent
                float percent_charge = node.storage(node_battery_charge{}) / 100.0;

                // if i'm terminating the current goal, i have to terminate goal for all nodes TODO: IS IT MEANT FOR ALL THE NODES IN THE PROCESS
                if (common::get<goal_code>(g) == node.storage(node_process_goal{}) && //i was running current goal in the process
                    common::get<goal_code>(g) == node.storage(node_external_goal{}) && //the robot was running current goal
                    ProcessingStatus::TERMINATING == node.storage(node_process_status{})) { //but now i'm terminating
                    return ends_processed_goal(CALL, p, g, s);
                }

                // if battery is empty, then stop at current position
                if (percent_charge <= 0.0) { //the battery is full discharged
                    battery_discharged_when_it_is_running(CALL, p, g, s);
                }

                // Battery charged
                else {
                    if (nt == node_type::ROBOT_MASTER) {
                        int c = 0;
                        if(node.storage(node_process_status{}) == ProcessingStatus::IDLE && (c = counter(CALL)) == 1){
                            std::cout << "Process GOAL " << c;
                            // I think here there should also be movement
                            // calculated(in a function), since we don't have a
                            // predetermined path, but get the end position from the
                            // goal
                            send_action_to_selected_node(CALL, nt, p, g, s);
                        }
                    }

                    if (nt == node_type::ROBOT_SLAVE) {
                        run_flocking(CALL, nt);
                        send_action_to_selected_node(CALL, nt, p, g, s);
                    }
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
        FUN void termination_logic(ARGS, status& s, goal_tuple_type const&, node_type nt) {
            bool terminatingMyself = s == status::terminated_output;
            bool terminatingNeigh = nbr(CALL, terminatingMyself, [&](field<bool> nt) {
                return any_hood(CALL, nt) or terminatingMyself;
                });
            bool exiting = all_hood(CALL, nbr(CALL, terminatingNeigh), terminatingNeigh);

            if (exiting) {
                s = status::border; 
            }
            else if (terminatingMyself) {
                s = status::internal_output;
            }
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
        FUN node_type init_main_fn(ARGS, int n_round, int number_of_masters) {
            node_type nt;

            // std::cout << "range: " << fcpp::coordination::comm << std::endl;

            if (AP_ENGINE_DEBUG) {
                std::cout << std::endl << std::endl;
                std::cout << "[node-" << node.uid << "] Time: " <<
                    std::chrono::time_point_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now()).time_since_epoch(
                        ).count() << endl;
            }

            #if defined(RUN_SIMULATION)
                if (node.uid < number_of_masters) {
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
            if (prefix == "RM")
            {
                node.storage(node_label_color{}) = fcpp::color(fcpp::GREEN);
            }
            else{
                node.storage(node_label_color{}) = fcpp::color(fcpp::BLACK);
            }
            node.storage(node_shadow_color{}) = fcpp::color(0x837E7CFF);
            node.storage(node_shadow_shape{}) = shape::sphere;

            old(CALL, robot_phase::IDLE, [&](robot_phase ph) {
                std::string rname = ROBOT_PREFIX + std::to_string(node.uid); // name is obtained from node ID
                node.storage(node_external_name{}) = rname;

                std::lock_guard lgr(RobotStatesMutex);
                // if robot is in the map
                if (RobotStatesMap.find(rname) != RobotStatesMap.end()) {
                    // for each status of the robot
                    for (RobotStatus rs : RobotStatesMap[rname]) {
                        // add offset to match with background
                        real_t pos_x_offset = node.storage(node_offset_pos_x{});
                        real_t pos_y_offset = node.storage(node_offset_pos_y{});
                        // set position
                        node.position() = make_vec(rs.pos_x + pos_x_offset, rs.pos_y + pos_y_offset, rs.pos_z);

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
                    // delete element from map, because it's already used and if there are new feedbacks, they will be added
                    RobotStatesMap.erase(rname);
                }

                return ph;
            });
        }

        // PROCESS MANAGEMENT

        //! @brief Spawn process from goal list acquired, which you get from the feedback.
        FUN spawn_result_type spawn_process(ARGS, node_type nt, ::vector<goal_tuple_type>& NewGoalsList, int n_round) {
            // process new goals, emptying NewGoalsList
            return coordination::spawn(CALL, [&](goal_tuple_type const& g) {
                status s = status::internal_output;

                field<int> test_nbr = fcpp::coordination::nbr(CALL, n_round);
                if (AP_ENGINE_DEBUG) {
                    std::cout << "test_nbr_in_process: " << test_nbr << std::endl;
                }

                // ACTION: ABORT GOAL
                // the two lines of check for process_goal and external_goal are because, we have simulation in AP and in gazebo(feedback)
                if (ABORT_ACTION == common::get<goal_action>(g) &&
                    node.storage(node_process_goal{}) == common::get<goal_code>(g) &&
                    node.storage(node_external_goal{}) == common::get<goal_code>(g)) {
                    manage_action_abort(CALL, g, &s);
                }

                // ACTION: REACH GOAL
                else if (GOAL_ACTION == common::get<goal_action>(g)) {
                    manage_action_goal(CALL, nt, g, &s, n_round);
                }

                termination_logic(CALL, s, g, nt);

                // Landing if status == border and pos_z > 0 
                if (nt == node_type::ROBOT_SLAVE && s == status::border && counter(CALL) == 1) {
                    action::ActionData action_data = {
                        .action = "LAND",
                        .goal_code = node.storage(node_process_goal{}),
                        .robot = get_real_robot_name(CALL, node.uid),
                        .pos_x = 0,
                        .pos_y = 0,
                        .pos_z = 0,
                        .orient_w = 0
                    };
                    action::manager::ActionManager::new_action(action_data);
                }
            return make_tuple(node.current_time(), s);
            }, NewGoalsList);
        }

        //! @brief Manage termination of the spawn processes.
        FUN void manage_termination(ARGS, node_type nt, spawn_result_type& r) {
            // if process was terminating and now it's terminated, we have to change state machine to IDLE 
            if (node.storage(node_process_status{}) == ProcessingStatus::TERMINATING) {
                // if process has been terminated, it isn't in the result map of spawn
                bool process_found = false;
                for (auto const& x : r) {
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

            // for testing purposes
            int nubmer_of_masters = 2;
            // INITIALIZE VARS
            std::vector<goal_tuple_type> NewGoalsList{};
            int n_round = fcpp::coordination::counter(CALL);

            node_type nt = init_main_fn(CALL, n_round, nubmer_of_masters);

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


            // Init Flocking
            initialization(CALL, nt);


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

        // TODO: refactor (cancellare le funzioni da togliere)
        FUN_EXPORT flocking_t = export_list<
            double,
            int,
            tuple<int, vec<3>>,
            vec<3>,
            tuple<bool, vec<3>>,
            tuple<int, int>,
            tuple <bool, double>,
            bool,
            tuple <bool, bool>
        >;

        //! @brief Export types used by the main function (update it when expanding the program).
        struct main_t : public export_list<
            any_connection_t,
            flocking_t,
            spawn_t<goal_tuple_type, status>,
            diameter_election_t<tuple<real_t, device_t>>
        > {};

    }
}

#endif // NODES_AP_ENGINE_H_
