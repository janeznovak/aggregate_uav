// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

/**
 * @file ap_engine.hpp
 * @brief AP code for the engine with multiple workers.
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
#include <optional>
#include "lib/goal_parser.h"

namespace fcpp
{

    namespace coordination
    {

        using namespace fcpp::coordination::tags;
        using namespace fcpp::option::data;
        using namespace fcpp::option;
        using namespace common;
        using namespace std::chrono;
        using spawn_result_type = std::unordered_map<goal_tuple_type, times_t, fcpp::common::hash<goal_tuple_type>>;

        // UTILS AP

        FUN bool isWorker(ARGS)
        {
            return node.storage(node_isWorker{}) ? true : false;
        }

        FUN bool isCharging(ARGS) {
            if(!isWorker(CALL)) {
                return node.storage(scout_isCharging{});
            } else return false;   
        }

        //! @brief Change color of node, passing in argument
        FUN void change_color(ARGS, fcpp::color color)
        {
            CODE
                node.storage(left_color{}) = fcpp::color(color);
            node.storage(right_color{}) = fcpp::color(color);
            node.storage(node_color{}) = fcpp::color(color);
        }

        //! @brief Blink color of node
        FUN void blink_computing_color(ARGS, int n_round)
        {
            CODE
                fcpp::color computing_color = fcpp::color(fcpp::coordination::computing_colors[n_round % 2]);
            change_color(CALL, computing_color);
        }

        //! @brief Get robot name from AP node_uid
        FUN string get_real_robot_name(ARGS, device_t node_uid)
        {
            CODE return get_robot_name(ROBOTS, static_cast<int>(node_uid));
        }

        //! @brief Update in the storage the tag "node_external_status_update_time"
        FUN void update_last_goal_update_time(ARGS)
        {
            CODE
                std::time_t now = std::time(nullptr);
            std::asctime(std::localtime(&now));
            node.storage(node_external_status_update_time{}) = now;
        }

        //! @brief Check if is passed "diff_time_ms" milliseconds from last REACHED change status
        FUN bool has_ms_passed_from_last_goal_update(ARGS, long diff_time_ms)
        {
            CODE
                std::time_t now = std::time(nullptr);
            milliseconds millis_to_check(diff_time_ms);
            std::time_t stored_time = node.storage(node_external_status_update_time{});

            milliseconds millis_computed = duration_cast<milliseconds>(system_clock::from_time_t(now) - system_clock::from_time_t(stored_time));
            return millis_computed >= millis_to_check;
        }

        //! @brief Add current goal to computing map
        FUN void add_goal_to_computing_map(ARGS, goal_tuple_type const &goal)
        {
            CODE
                node.storage(node_process_computing_goals{})[get<goal_code>(goal)] = goal;
        }

        //! @brief Remove current goal to computing map
        FUN void remove_goal_from_computing_map(ARGS, std::string goal_code)
        {
            CODE
                node.storage(node_process_computing_goals{})
                    .erase(goal_code);
        }

        //! @brief Send "stop" command to robot and change "node_process_status" after it
        FUN void send_stop_command_to_robot(ARGS, string action, device_t node_uid, goal_tuple_type const &goal, ProcessingStatus ps)
        {
            CODE
                std::string robot_chosen = get_real_robot_name(CALL, node_uid);
            std::cout << "ROBOT_MASTER " << robot_chosen << " is chosen for receiving ABORT command for goal " << get<goal_code>(goal) << endl;
            std::cout << endl;

            action::ActionData action_data = {
                .action = action,
                .goal_code = get<goal_code>(goal),
                .robot = robot_chosen,
                .pos_x = get<goal_pos_x>(goal),
                .pos_y = get<goal_pos_y>(goal),
                .pos_z = get<goal_pos_z>(goal),
                .orient_w = get<goal_orient_w>(goal)};

            action::manager::ActionManager::new_action(action_data);

            // clear goal info if in idle
            if (ProcessingStatus::IDLE == ps)
            {
                node.storage(node_process_goal{}) = "";
            }

            // clear processing status
            node.storage(node_process_status{}) = ps;
        }

        // Flocking Utils

        bool compare(double a, double b)
        {
            return a < b;
        }

        FUN bool isActive(ARGS){
            return (node.storage(node_active{}) == 1) ? true : false;
        }

        void log_error(string uid, string time, double error)
        {
            std::string out_path = string(OUTPUT_FOLDER_BASE_PATH) + string("from_ap/to_log/");
            create_folder_if_not_exists(out_path);

            std::ofstream file;
            std::string uniqueFilename = out_path + "/" + uid + "_error_log.txt";

            file.open(uniqueFilename, std::ios::app);

            if (file.is_open())
            {
                file << time << "," << std::to_string(error) << std::endl;
                file.close();
            }
            else
            {
                std::cout << "Error log file." << std::endl;
            }
        }

        // STATE MACHINE

        //! @brief Manage state machine when battery is discharged
        FUN void manage_battery_discharged_node(ARGS)
        {
            CODE
                change_color(CALL, fcpp::color(fcpp::coordination::discharged_color));
        }

        //! @brief Manage state machine when robot is running a goal
        FUN void manage_running_goal_status(ARGS)
        {
            CODE
                change_color(CALL, fcpp::color(fcpp::coordination::running_color));
        }

        //! @brief Manage state machine when robot has reached the goal
        FUN void manage_reached_goal_status(ARGS)
        {
            CODE
                fcpp::color new_color;
            // and previous state is NOT REACHED: new color is "reached" and deletes goal storage
            if (feedback::GoalStatus::REACHED != node.storage(node_external_status{}))
            {
                new_color = fcpp::color(fcpp::coordination::reached_goal_color);
                // set to terminating processing status
                node.storage(node_process_status{}) = ProcessingStatus::TERMINATING;
                // update time
                update_last_goal_update_time(CALL);

                // and if process node is SELECTED, then AP has selected new node and simulation is ready to start
            }
            else if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED)
            {
                new_color = fcpp::color(fcpp::coordination::running_color);

                // otherwise: new color is "reached"
            }
            else
            {
                new_color = fcpp::color(fcpp::coordination::reached_goal_color);
            }
            change_color(CALL, new_color);
        }

        //! @brief Manage state machine when robot reported an UNKNOWN error
        FUN void manage_unknown_goal_status(ARGS)
        {
            CODE
                fcpp::color new_color;
            // and if process node is SELECTED, then AP has selected new node and simulation is ready to start
            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED)
            {
                new_color = fcpp::color(fcpp::coordination::running_color);
            }
            else
            {
                new_color = fcpp::color(fcpp::coordination::idle_color);
                // update time
                update_last_goal_update_time(CALL);
            }
            change_color(CALL, new_color);
        }

        //! @brief Manage state machine when robot reported an abort of the current goal
        FUN void manage_aborted_goal_status(ARGS)
        {
            CODE
                fcpp::color new_color;
            // and if process node is SELECTED, then AP has selected new node and simulation is ready to start
            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED)
            {
                new_color = fcpp::color(fcpp::coordination::running_color);
            }
            else
            {
                new_color = fcpp::color(fcpp::coordination::aborted_goal_color);
                // update time
                update_last_goal_update_time(CALL);
            }
            change_color(CALL, new_color);
        }

        //! @brief Manage state machine when robot reported a FAILED error
        FUN void manage_failed_goal_status(ARGS)
        {
            CODE
                fcpp::color new_color;
            // and previous state is not FAILED (first time of FAILED status): change internal status to IDLE
            if (feedback::GoalStatus::FAILED != node.storage(node_external_status{}))
            {
                new_color = fcpp::color(fcpp::coordination::failed_goal_color);
                // resetting processing status
                node.storage(node_process_status{}) = ProcessingStatus::IDLE;
                // update time
                update_last_goal_update_time(CALL);

                // or previous state is FAILED (it's NOT the first time of FAILED status)
            }
            else
            {
                // and if process node is SELECTED, then AP has selected new node and simulation is ready to start
                if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED)
                {
                    new_color = fcpp::color(fcpp::coordination::running_color);
                }
                else
                {
                    new_color = fcpp::color(fcpp::coordination::failed_goal_color);
                }
            }
            change_color(CALL, new_color);
        }

        //! @brief Manage state machine when robot has not yet executed a goal
        FUN void manage_no_goal_status(ARGS)
        {
            CODE
                fcpp::color new_color;
            // and if process node is SELECTED, then AP has selected new node and simulation is ready to start
            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED)
            {
                new_color = fcpp::color(fcpp::coordination::running_color);
            }
            else
            {
                new_color = fcpp::color(fcpp::coordination::idle_color);
            }
            change_color(CALL, new_color);
        }

        // FLOCKING

        //! @brief controllo l'indice e nel caso non fosse valido lo riassegno...
        FUN void fixIndex(ARGS)
        {
            using namespace tags;
            if (!isWorker(CALL))
            {
                if ((get<1>(node.storage(node_indexSlave{}))) > 0)
                {
                    field<tuple<int, int>> f = nbr(CALL, node.storage(node_indexSlave{}));
                    tuple<int, int> check = max_hood(CALL, f);
                    if (get<1>(node.storage(node_indexSlave{})) == get<1>(check) && get<0>(node.storage(node_indexSlave{})) != get<0>(check))
                    {
                        node.storage(node_fixIndex{}) = true;
                        // if (!(node.storage(node_secondReturn{})))
                        // {
                        //     get<1>(node.storage(node_indexSlave{})) += 1;
                        // }
                        // else
                        // {
                        //     get<1>(node.storage(node_indexSlave{})) -= 1;
                        // }
                        get<1>(node.storage(node_indexSlave{})) += 1;
                    }
                }
            }
        }

        // to calculate where in the circumference the slave is
        FUN void calculateMyCorner(ARGS)
        {
            using namespace tags;
            // std::cout << "Calculating my corner before " << node.storage(node_vecMyVersor{}) << std::endl;
            // if it has a master
            if (get<0>(node.storage(node_posMaster{})))
            {
                double myRadiant = (get<1>(node.storage(node_indexSlave{}))) * ((2 * pi) / node.storage(node_numberOfSlave{}));
                node.storage(node_myRadiant{}) = myRadiant;
                // my radiant
                // std::cout << "Calculating my radiant " << node.storage(node_myRadiant{}) << std::endl;
                double sine_value = std::sin(myRadiant);
                double cosine_value = std::cos(myRadiant);
                double x = sine_value * distanceMasterSlave;
                double y = cosine_value * distanceMasterSlave;
                if (node.storage(node_fixIndex{}))
                {
                    x = sine_value * distanceCircularCrown;
                    y = cosine_value * distanceCircularCrown;
                }
                vec<3> vecMyRadiant = make_vec(x, y, 0);
                node.storage(node_vecMyRadiant{}) = vecMyRadiant;
                // this is my radiant
                // std::cout << "Calculating my vec_radiant " << node.storage(node_vecMyRadiant{}) << std::endl;

                /**traslazione del cerchio attorno al master*/
                vec<3> dist = get<1>(node.storage(node_posMaster{})) - node.position();
                vec<3> versore = vecMyRadiant + dist;
                node.storage(node_vecMyVersor{}) = versore;
                // this is my versore
                // std::cout << "Calculating my vec_versore " << node.storage(node_vecMyVersor{}) << std::endl;
                // std::cout << "My node_indexSlave " << get<1>(node.storage(node_indexSlave{})) << std::endl;
                // std::cout << "Calculating my corner after " << node.storage(node_vecMyVersor{}) << std::endl;
                // if (!(isnan(versore[0]) && isnan(versore[1]))) {
                // node.propulsion() = versore / norm(versore);
                // }
                // node.propulsion() += node.propulsion() * incrementAcceleration;
            }
        }

        //! @brief if slave nodes exit the circumference, reset the indices of the slaves within the circumference that exceed the maximum index...
        FUN bool decrementIndex(ARGS)
        {
            using namespace tags;
            int current_slaves = max_hood(CALL, nbr(CALL, node.storage(node_numberOfSlave{})));
            if (get<1>(node.storage(node_indexSlave{})) > current_slaves && current_slaves != 0)
            {
                get<1>(node.storage(node_indexSlave{})) = 1;
                return true;
            }
            return false;
        }

        FUN void errorCalculator(ARGS)
        {
            using namespace tags;
            if (get<0>(node.storage(node_posMaster{})))
            {
                double theta = (get<1>(node.storage(node_indexSlave{}))) * ((2 * pi) / node.storage(node_numberOfSlave{}));
                double x = std::sin(theta) * distanceMasterSlave;
                double y = std::cos(theta) * distanceMasterSlave;
                vec<3> vecTheta = make_vec(x, y, 0);
                vec<3> exactPosition = vecTheta + get<1>(node.storage(node_posMaster{}));
                double error = distance(exactPosition, node.position());
                double normalizedError = error / distanceMasterSlave; // Normalizzazione dell'errore
                node.storage(position_error{}) = error;
                log_error(
                    std::to_string(node.uid),
                    std::to_string(time_point_cast<milliseconds>(system_clock::now()).time_since_epoch().count()),
                    error);
            }
        }

        FUN void collisionAvoidance(ARGS)
        {
            using namespace tags;
            if (!isWorker(CALL))
            {
                if (get<0>(node.storage(node_posMaster{})))
                {
                    bool flag = compare(distance(get<1>(node.storage(node_posMaster{})), node.position()), minDistance);
                    vec<3> elasticMaster = make_vec(0, 0, 0);
                    vec<3> v = get<1>(node.storage(node_posMaster{})) - node.position();
                    if (!node.storage(node_fixIndex{}))
                    {
                        elasticMaster = v * (1 - distanceMasterSlave / (norm(v))) * hardnessMasterSlave;
                    }
                    else
                    {
                        elasticMaster = v * ((1 - distanceCircularCrown / (norm(v)))) * hardnessCircularCrown;
                    }
                    if (flag)
                    {
                        //! Viene considerata come limite critico di livello 2 la distanza minima consentita
                        elasticMaster += elasticMaster * incrementForce;
                    }
                    node.storage(node_collisionAvoidanceMaster{}) = elasticMaster;

                    //! Viene considerata come limite critico di livello 1 il 90% della suddivisione in parti uguali per permettere
                    //! un margine d'errore del 10%
                    double distanceSlaveSlave = (2 * distanceMasterSlave * pi) / node.storage(node_numberOfSlave{});

                    tuple<bool, vec<3>> elasticSlave = sum_hood(CALL, map_hood([](vec<3> v, double d, double l, double constAvoid)
                                                                               {
                        tuple<bool, vec<3>> t = make_tuple(false, make_vec(0, 0, 0));
                        double criticalLimit = l * 0.9;
                        if (d < criticalLimit) {
                            get<1>(t) = v * ((1 - l / (norm(v))) * hardnessSlaveSlave);
                            //! Viene considerata come limite critico di livello 2 la distanza minima consentita
                            if (d < constAvoid) {
                                get<1>(t) += get<1>(t) * incrementForce;
                            }
                        }
                        return t; }, node.nbr_vec(), node.nbr_dist(), distanceSlaveSlave, minDistance),
                                                                tuple<bool, vec<3>>{});
                    node.storage(node_collisionAvoidanceSlaves{}) = get<1>(elasticSlave);
                    if (node.storage(node_collisionAvoidanceSlaves{}) != make_vec(0, 0, 0))
                    {
                        node.storage(node_flagDistance{}) = true;
                    }
                    else
                    {
                        node.storage(node_flagDistance{}) = false;
                    }

                    //! Viene semplicemente controllato che i valori calcolati siano assegnabili, ovvero !NaN || !inf
                    if ((!(isnan(node.storage(node_collisionAvoidanceMaster{})[0])) && !(std::isinf(node.storage(node_collisionAvoidanceMaster{})[0]))) ||
                        (!(isnan(node.storage(node_collisionAvoidanceMaster{})[1])) && !(std::isinf(node.storage(node_collisionAvoidanceMaster{})[1]))))
                    {

                        node.storage(node_vecMyVersor{}) += node.storage(node_collisionAvoidanceMaster{});
                        if ((!(isnan(node.storage(node_collisionAvoidanceSlaves{})[0])) && !(std::isinf(node.storage(node_collisionAvoidanceSlaves{})[0]))) ||
                            (!(isnan(node.storage(node_collisionAvoidanceSlaves{})[1])) && !(std::isinf(node.storage(node_collisionAvoidanceSlaves{})[1]))))
                        {

                            node.storage(node_vecMyVersor{}) += node.storage(node_collisionAvoidanceSlaves{});
                        }
                    }
                }
            }
        }

        FUN void run_flocking(ARGS)
        {
            CODE
                /**controllo nel caso degli indici si sovrapponessero*/
                field<tuple<int, int>>
                    identifierWrongIndex = nbr(CALL, node.storage(node_indexSlave{}));

            // Aggiungere a true la batteria è a 0
            bool flagIndex = all_hood(CALL, map_hood([](tuple<int, int> t, tuple<int, int> myValue)
                                                     {
                if (get<1>(myValue) == get<1>(t) && get<0>(myValue) != get<0>(t)) {
                    return false;
                }
                else {
                    return true;
                } }, identifierWrongIndex, node.storage(node_indexSlave{})));

            // if (!(flagIndex && decrementIndex(CALL)))
            // {
            //     fixIndex(CALL);
            // }
            // else
            // {
            //     node.storage(node_fixIndex{}) = false;
            // }

            if (!isWorker(CALL))
            {
                // instead calculateMyCorner we should have the slave move to a
                // position of interest based on something(maybe master, maybe goal from
                // a user, ...)
                calculateMyCorner(CALL);
                //! Sistema di collision avoidance
                collisionAvoidance(CALL);
                errorCalculator(CALL);
            }

            if (!isWorker(CALL))
            {
                if (!(get<0>(node.storage(node_posMaster{}))))
                {
                    node.storage(node_startPosition{}) = node.position();
                }
                else
                {
                    node.storage(node_startPosition{}) = node.position();
                }
            }
        }

        FUN void updateWorker(ARGS)
        {
            CODE
                infoWorkerType infoWorker = make_tagged_tuple<infoW_active, infoW_need, infoW_nodeDistance, infoW_position, infoW_nodeId>(false, INT_MIN, INT_MAX, make_vec(0, 0, 0), node.uid);
            if (isWorker(CALL))
            {
                infoWorker = make_tagged_tuple<infoW_active, infoW_need, infoW_nodeDistance, infoW_position, infoW_nodeId>(true, node.storage(scout_need{}), INT_MAX, node.position(), node.uid);
            }
            else
            {
                // TODO: maybe have a use case for extra scouts
            }
            node.storage(expected_dist_worker_scout{}) = distanceMasterSlave;

            // // TODO: maybe do a out of order simulation

            int workerId = node.storage(scout_curr_worker{});

            field<infoWorkerType> infoWorkerField = nbr(CALL, infoWorker);

            // Used to retrive the info of the current scout's worker. I think the acc is a placeholder again, and is used if the scout doesn't have a worker
            infoWorkerType currWorker = fold_hood(CALL, [&](infoWorkerType val, infoWorkerType acc) { 
                if(get<infoW_nodeId>(val) == workerId) {
                    return val;
                } else return acc;
            }, infoWorkerField, make_tagged_tuple<infoW_active, infoW_need, infoW_nodeDistance, infoW_position, infoW_velocity, infoW_nodeId>(false, INT_MIN, INT_MAX, make_vec(0, 0, 0), make_vec(0, 0, 0), node.uid));   

            // Update the scout's info on the worker
            if(get<infoW_active>(currWorker)) {
                node.storage(node_posMaster{}) = make_tuple(get<infoW_active>(currWorker), get<infoW_position>(currWorker));
            }

            // Update the distance of the scout to the workers(multiple, since we used nbr and we have multiple workers)
            field<infoWorkerType> updatedInfoWorkerField = map_hood([&](infoWorkerType workerData) {
                double dist = -(distance(get<infoW_position>(workerData), node.position()));

                get<infoW_nodeDistance>(workerData) = dist;

                return workerData;
            }, infoWorkerField);

            infoWorkerType newWorkerInfo = max_hood(CALL, updatedInfoWorkerField); // gets the worker with the smallest distance to the scout
            // smallest because, we negate the distance in the map_hood function, so the smallest distance is then the largest value

            if (node.storage(node_countRound{}) > 400 && get<infoW_active>(newWorkerInfo)) // check with Gianluca if we need the roundcount
            {
            //     // only for scouts
                if (!isWorker(CALL))
                {
                    // find the closest scout. If there is a need for a new scout, we will reassign one, otherwise nothing
                    // TODO: would it be better here to first check if the worker actually needs a new scout?

                    tuple<vec<3>, int> closestScout = make_tuple(node.position(), -1);

                    split(CALL, node.storage(scout_curr_worker{}), [&]() { // executes only in the network of the given key, which in this case is the current worker of the current scout
                        tuple<vec<3>, int> tupleScout = make_tuple(node.position(), node.uid);

                        field<tuple<vec<3>, int>> fieldScout = nbr(CALL, tupleScout); // field of scouts(I think only the scouts of the network of the current worker), since the worker cannot get into this code

                        closestScout = fold_hood(CALL, [&](tuple<vec<3>, int> val, tuple<vec<3>, int> acc) { // acc is the starting value, and after the return what is returned is the new acc

                            double accDist = distance(get<infoW_position>(newWorkerInfo), get<0>(acc));
                            double valDist = distance(get<infoW_position>(newWorkerInfo), get<0>(val));

                            return valDist < accDist ? val : acc;
                        }, fieldScout, make_tuple(node.position(), node.uid));
                    });

                    if(node.storage(scout_curr_worker{}) < 0 || (get<infoW_need>(newWorkerInfo) > 0 && node.uid == get<1>(closestScout) && get<infoW_need>(currWorker) < get<infoW_need>(newWorkerInfo) - 1)) {
                        if(!isWorker(CALL)) node.storage(scout_curr_worker{}) = get<infoW_nodeId>(newWorkerInfo);

                        // change the worker for current scout
                        node.storage(node_posMaster{}) = make_tuple(get<infoW_active>(newWorkerInfo), get<infoW_position>(newWorkerInfo));
                    } 
                }
                
            }
            
        }

        FUN void updateFollowersCount(ARGS) {
            if(isActive(CALL)) {
                int num_scouts = count_hood(CALL) - 1;

                if(isWorker(CALL)) node.storage(scout_need{}) = node.storage(required_scouts{}) - num_scouts;
                node.storage(node_numberOfSlave{}) = num_scouts;
            }
        }

        FUN void assignScout(ARGS) {
            /**When a node identifies the position of the worker, it is assigned an index that corresponds
             * to the order of arrival within the circle.*/
            if (isActive(CALL)) {
                std::cout << "For node: " << node.uid << " the current worker is: " << node.storage(scout_curr_worker{}) << std::endl;
                std::cout << "My index before assign: " << get<1>(node.storage(node_indexSlave{})) << std::endl;

                tuple<int, int> scoutVal = make_tuple(node.storage(scout_curr_worker{}), get<1>(node.storage(node_indexSlave{}))); // id of the scouts current worker and the index of the scout
                field<tuple<int, int>> fieldScoutVal = nbr(CALL, scoutVal); // just a field of the scoutVal tuple of the neighbors

                tuple<int, int> currMaxIndex = fold_hood(CALL, [&](tuple<int, int> val, tuple<int, int> acc) {
                    if(get<1>(val) > get<1>(acc) && node.storage(scout_curr_worker{}) == get<0>(val) && !isWorker(CALL)) { // check if the current node has a higher value than the previous max value, that the worker of both is the same and that the current node is not the worker
                        return val;
                    } else return acc;
                }, fieldScoutVal, make_tuple(node.storage(scout_curr_worker{}), 0));


                int maxIndex = nbr(CALL, 0, [&](field<int> indexes) {
                    maxIndex = get<1>(currMaxIndex); // globaly max index of a scout
                    if (get<1>(node.storage(node_indexSlave{})) == 0 && !isWorker(CALL)) { // if the scout has no index, we assign one more than the max
                        return maxIndex + 1;
                    } else {
                        return maxIndex;
                    }
                });
                // the maxindex
                // std::cout << "Max index: " << maxIndex << std::endl;
                // Assign the found value, but just if the scout has no index
                if ((get<1>(node.storage(node_indexSlave{}))) == 0 && !isWorker(CALL)) {
                    // I am setting the index of the scout to the max index
                    // std::cout << "Setting index to: " << maxIndex << "for node: " << node.uid << std::endl;
                    std::cout << "Changing index for node: " << node.uid << " from: " << get<1>(node.storage(node_indexSlave{})) << " to: " << maxIndex << std::endl;
                    node.storage(node_indexSlave{}) = make_tuple(node.uid, maxIndex);
                }
                std::cout << "My index after assign: " << get<1>(node.storage(node_indexSlave{})) << std::endl;
            } 
        }

        /**
         * Check if my index is equal to that of a neighbor.
         * Returns TRUE if it finds duplicates of its index
        */
        FUN bool checkDuplicateIndex(ARGS) { CODE
            /**Check in case the indexes overlap*/
            bool duplicateExists = false;
            if (!isWorker(CALL) && isActive(CALL)) { // TODO: could we remove the !isWorker(CALL) check, so that the master propagates the indexes of the slaves as well
                field<tuple<int, int>> identifierWrongIndex = nbr(CALL, node.storage(node_indexSlave{}));
                duplicateExists = any_hood(CALL, map_hood([](tuple<int, int> t, tuple<int, int> myValue) {
                    if (get<1>(myValue) == get<1>(t) && get<0>(myValue) != get<0>(t)) { // check if current node has the same index as the one from neighbour and that the node is not the same one, so you don't compare me to me
                        return true;
                    }else{
                        return false;
                    }
                }, identifierWrongIndex, node.storage(node_indexSlave{})));
            }
            return duplicateExists;
        }

        FUN void correctIndexes(ARGS) {
            if (isActive(CALL) && !isCharging(CALL)) 
            {
                // Check for duplicate indexes
                bool duplicatedIndexes = split(CALL, node.storage(scout_curr_worker{}), [&]() {return checkDuplicateIndex(CALL);}); // check for duplicates in the network of the current worker
                
                if (duplicatedIndexes)
                {
                    std::cout << "Found duplicate indexes for node: " << node.uid << " with index: " << get<1>(node.storage(node_indexSlave{})) << std::endl;
                    // If there are duplicates, adjust the indexes accordingly
                    split(CALL, node.storage(scout_curr_worker{}), [&]() {return fixIndex(CALL);}); // do for the network of the current worker
                } else {
                    node.storage(node_fixIndex{}) = false;
                }       

                // Decrement indexes higher than the current number of scouts
                // In the next round, they will be seen as duplicates and will be fixed
                split(CALL, node.storage(scout_curr_worker{}), [&]() {return decrementIndex(CALL);}); // do for the network of the current worker
            }
        }

        // AP PROCESS

        //! @brief A robot has reached a goal and now try to terminate the process
        FUN process_tuple_type ends_processed_goal(ARGS, process_tuple_type &process, goal_tuple_type const &goal, status *status_end)
        {
            CODE

                    std::cout
                << "Robot " << node.uid << " is trying to terminate goal " << get<goal_code>(goal) << endl;
            std::cout << endl;

            *status_end = status::terminated_output; // stop propagation
            return process;
        }

        //! @brief A robot has discharged the battery, so AP send a stop command
        FUN void battery_discharged_when_it_is_running(ARGS, process_tuple_type &p, status *status_discharged)
        {
            CODE
                *status_discharged = status::border; // listen neighbours, but not send messages
        }

        //! @brief Send a GOAL action to selected node and update the AP state machine of the robot to SELECTED
        FUN void send_action_to_selected_node(ARGS, goal_tuple_type const &goal)
        {
            CODE
                std::string robot_chosen = get_real_robot_name(CALL, node.uid);

            if (isWorker(CALL))
            {
                // std::cout << "Robot " << robot_chosen << " is chosen for goal " << get<goal_code>(goal) << endl;
                // print the goal_code_robot_id and node.uid
                // std::cout << "Goal code robot id: " << goal_code_robot_id << " Node uid: " << node.uid << endl;
                // std::cout << endl;

                // set processing status to SELECTED
                node.storage(node_process_status{}) = ProcessingStatus::SELECTED;

                // save goal
                node.storage(node_process_goal{}) = get<goal_code>(goal);

                // send action to file
                action::ActionData action_data = {
                    .action = get<goal_action>(goal),
                    .goal_code = get<goal_code>(goal),
                    .robot = robot_chosen,
                    .pos_x = get<goal_pos_x>(goal),
                    .pos_y = get<goal_pos_y>(goal),
                    .pos_z = get<goal_pos_z>(goal),
                    .orient_w = get<goal_orient_w>(goal)};
                action::manager::ActionManager::new_action(action_data);
                // print the position of x
                // std::cout << "Position x: " << get<goal_pos_x>(goal) << endl;
            }
            else
            {
                // send action to file a.k.a send the goal to the scout/slave
                action::ActionData action_data = {
                    .action = get<goal_action>(goal),
                    .goal_code = get<goal_code>(goal),
                    .robot = robot_chosen,
                    .pos_x = (float)node.storage(node_vecMyVersor{})[0],
                    .pos_y = (float)node.storage(node_vecMyVersor{})[1],
                    .pos_z = (float)node.storage(node_vecMyVersor{})[2],
                    .orient_w = 0.0};
                action::manager::ActionManager::new_action(action_data);
            }
        }

        // ACTION

        //! @brief Manage when the user has requested an ABORT of a goal
        FUN void manage_action_abort(ARGS, goal_tuple_type const &goal, status *status_abort)
        {
            CODE
                    std::cout
                << "Process ABORT " << get<goal_code>(goal) << " in node " << node.uid << " with status "
                << node.storage(node_process_status{}) << endl;
            // if and only if robot is in status RUNNING, sends stop command to robot
            if (node.storage(node_process_status{}) == ProcessingStatus::SELECTED)
            {
                send_stop_command_to_robot(CALL, "ABORT", node.uid, goal, ProcessingStatus::TERMINATING);

                std::cout << "Robot " << node.uid << " is trying to terminate ABORT " << get<goal_code>(goal) << endl;
            }
            else
            {
                if (AP_ENGINE_DEBUG)
                {
                    std::cout << "Robot " << node.uid << " is waiting to terminate ABORT " << get<goal_code>(goal) << endl;
                }
            }
            *status_abort = status::terminated_output; // stop propagation
        }

        //! @brief Manage when the user has requested a new GOAL
        FUN void manage_action_goal(ARGS, goal_tuple_type const &goal, status *status_goal, int n_round)
        {
            CODE

                // std::cout << "Process GOAL " << get<goal_code>(goal) << ", action " << get<goal_action>(goal) << " in node " << node.uid << endl;
                add_goal_to_computing_map(CALL, goal);

            old(CALL, make_tagged_tuple<goal_code>(get<goal_code>(goal)), [&](process_tuple_type process)
                {

                // compute charge of battery in percent
                float percent_charge = node.storage(node_battery_charge{}) / 100.0;

                // if i'm terminating the current goal, i have to terminate goal for all nodes TODO: IS IT MEANT FOR ALL THE NODES IN THE PROCESS
                if (get<goal_code>(goal) == node.storage(node_process_goal{}) && //i was running current goal in the process
                    get<goal_code>(goal) == node.storage(node_external_goal{}) && //the robot was running current goal
                    ProcessingStatus::TERMINATING == node.storage(node_process_status{})) { //but now i'm terminating
                    return ends_processed_goal(CALL, process, goal, status_goal);
                }

                // if battery is empty, then stop at current position
                if (percent_charge <= 0.0) { //the battery is full discharged
                    battery_discharged_when_it_is_running(CALL, process, status_goal);
                }

                // Battery charged
                else {
                    if (isWorker(CALL)) {
                        int c = 0;
                        int goal_code_robot_id = std::stoi(get_robot_id_from_goal_code(get<goal_code>(goal)));
                        if(node.storage(node_process_status{}) == ProcessingStatus::IDLE && node.uid == goal_code_robot_id && (c = counter(CALL)) == 1){
                            std::cout << "Process GOAL " << c;
                            // I think here there should also be movement
                            // calculated(in a function), since we don't have a
                            // predetermined path, but get the end position from the
                            // goal
                            send_action_to_selected_node(CALL, goal);
                        }
                    }

                    if (!isWorker(CALL)) {
                        // std::cout << "I am running the flocking" << std::endl;
                        run_flocking(CALL);
                        // print out what is sent to send_action_to_selected_node, so the goal and the process
                        std::cout << "Goal code: " << get<goal_code>(goal) << " Process: " << process << std::endl;
                        if (std::stoi(get_robot_id_from_goal_code(get<goal_code>(goal))) == node.storage(scout_curr_worker{})) 
                        {
                            send_action_to_selected_node(CALL, goal);
                        }
                    }
                }
                // blinking colors if not running
                if (get<goal_code>(goal) != node.storage(node_process_goal{}) &&
                    node.storage(node_process_status{}) != ProcessingStatus::SELECTED) {
                    blink_computing_color(CALL, n_round);
                }

                // TODO
                return process; });
        }

        //! @brief Termination logic using share (see SHARE termination in ACSOS22 paper)
        FUN void termination_logic(ARGS, status &status_terminate, goal_tuple_type const &)
        {
            bool terminatingMyself = status_terminate == status::terminated_output;
            bool terminatingNeigh = nbr(CALL, terminatingMyself, [&](field<bool> nt)
                                        { return any_hood(CALL, nt) or terminatingMyself; });
            bool exiting = all_hood(CALL, nbr(CALL, terminatingNeigh), terminatingNeigh);

            if (exiting)
            {
                status_terminate = status::border;
            }
            else if (terminatingMyself)
            {
                status_terminate = status::internal_output;
            }
        }

        //! @brief Read new goals from shared variable and insert them in NewGoalsList
        void read_new_goals(std::vector<goal_tuple_type> &NewGoalsList, std::optional<std::string> source)
        {
            std::vector<InputGoal> InputGoalsBySource;

            // if source is specified
            if (source.has_value())
            {
                // keep only goals with source = node_ext_name
                std::copy_if(InputGoalList.begin(), InputGoalList.end(), std::back_inserter(InputGoalsBySource), [&](InputGoal ig)
                             { return get_robot_id_from_goal_code(ig.goal_code) == source.value(); });
            }
            else
            {
                // otherwise, copy all InputGoal
                InputGoalsBySource = InputGoalList;
            }

            std::lock_guard lgg(GoalMutex);
            auto map_op = [](InputGoal ig)
            {
                return make_tagged_tuple<goal_action, goal_code, goal_pos_x, goal_pos_y, goal_pos_z, goal_orient_w, goal_source, goal_priority, goal_subcode>(
                    ig.action,
                    ig.goal_code,
                    ig.pos_x,
                    ig.pos_y,
                    ig.pos_z,
                    ig.orient_w,
                    ig.source,
                    ig.priority,
                    ig.subcode);
            };
            std::transform(InputGoalsBySource.begin(), InputGoalsBySource.end(), std::back_inserter(NewGoalsList), map_op);

            // delete only goals with source = node_ext_name
            if (source.has_value())
            {
                // std::cout << "I am erasing" << std::endl;
                // std::cout << "This is the source value" << source.value() << std::endl;
                InputGoalList.erase(
                    std::remove_if(InputGoalList.begin(), InputGoalList.end(), [&](InputGoal ig)
                                   { return get_robot_id_from_goal_code(ig.goal_code) == source.value(); }),
                    InputGoalList.end());
            }
            else
            {
                // std::cout << "I am clearing" << std::endl;
                InputGoalList.clear();
            }
        }

        //! @brief Acquire new goals from storage, in according with node_type.
        FUN void acquire_new_goals(ARGS, std::vector<goal_tuple_type> &NewGoalsList)
        {
            CODE
                read_new_goals(NewGoalsList, std::optional<string>(std::to_string(node.uid)));
        }

        // MAIN FUNCTIONS

        //! @brief Initialize MAIN function, selecting correct node_type, run only ONCE per simulation
        FUN void init_main_fn(ARGS, int n_round, int number_of_masters)
        {
            // std::cout << "range: " << fcpp::coordination::comm << std::endl;

            if (AP_ENGINE_DEBUG) {
                std::cout << std::endl << std::endl;
                std::cout << "[node-" << node.uid << "] Time: " <<
                    std::chrono::time_point_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now()).time_since_epoch(
                        ).count() << endl;
            }

            // set node type in the storage
            #if defined(RUN_SIMULATION)
                if (node.uid < number_of_masters)
                {
                    node.storage(node_isWorker{}) = true;
                    node.storage(node_label_text{}) = "RM." + std::to_string(node.uid);

                    // TODO: probably need the worker_needs so that different workers have different amount of workers
                    node.storage(required_scouts{}) = 2; // FOR NOW IT'S JUST SET TO TO, BUT IT SHOULD BE CHANGED SO THAT A USER CAN SET THE INITIAL VALUE
                    node.storage(scout_need{}) = 2 - nWorkerScout; // TODO: change this and original required scouts so they read from an array, where the index is the node.uid
                    node.storage(original_required_scouts{}) = 2;
                    node.storage(scout_curr_worker{}) = -1; // the worker is not assigned to a scout
                    node.storage(node_numberOfSlave{}) = nWorkerScout; // number of scouts assigned to each worker
                }
                else
                {
                    node.storage(node_isWorker{}) = false;
                    node.storage(node_label_text{}) = "RS." + std::to_string(node.uid);

                    for (int i = 1; i <= number_of_masters; i++)
                    {
                        int val = (node.uid - (number_of_masters - 1)) / (nWorkerScout * i);

                        // Assign the scout to the worker(to it's worker, since this function is run only once and each worker has it's own scouts)
                        if(val == 0 || (node.uid - (number_of_masters - 1)) == nWorkerScout * i)
                        {
                            node.storage(scout_curr_worker{}) = i - 1;
                            // print the scout_curr_worker with my worker is: scout_curr_worker
                            // std::cout << "Scout " << node.uid << "assigned to worker " << node.storage(scout_curr_worker{}) << std::endl;
                            break;
                        }
                    }

                    // TODO: the battery is not use yet, can be in the future
                    node.storage(scout_need{}) = -1;
                    node.storage(scout_isCharging{}) = false;
                    node.storage(scout_battery_percentage{}) = 100;
                    node.storage(scout_min_battery_percaentage{}) = 15;

                    // TODO: here set the battery discharge rate, depeneding on the type of the battery
                    // node.storage(scout_battery_discharge_rate{}) = 0.1; // this is just a placeholder for now, for real test it should be a
                    // different rate, based on what user chose


                    
                }
            #elif defined(RUN_EMBEDDED)
                node.storage(node_isWorker{}) = node.uid == 0 ? true : false;
            #endif
            if (AP_ENGINE_DEBUG)
            {
                std::cout << "MAIN FUNCTION in node " << node.uid << " of type " << (isWorker(CALL) ? "worker" : "scout") << endl;
            }

            // TODO: this might be needs to be used more than just once in the debug mode
            field<int> test_nbr = fcpp::coordination::nbr(CALL, n_round);
            if (AP_ENGINE_DEBUG)
            {
                std::cout << "test_nbr_main_fn: " << test_nbr << "; nbr_uid: " << nbr_uid(CALL) << std::endl;
            }

            // set parameters that will not change and mark that they have been initialised
            node.storage(node_size{}) = NODE_SIZE;
            node.storage(node_label_size{}) = LABEL_SIZE;
            node.storage(node_shape{}) = shape::sphere;
            node.storage(node_shadow_color{}) = fcpp::color(0x837E7CFF);
            node.storage(node_shadow_shape{}) = shape::sphere;
            node.storage(expected_dist_worker_scout{}) = distanceMasterSlave;


            node.storage(node_set{}) = true;
        }

        //! @brief Initialize variables (storage, etc...) of a robot using feedback data.
        FUN void apply_feedback_to_ap(ARGS)
        {
            // std::cout << "Robot " << prefix << node.uid << " initialized" << std::endl;

            // set position in AP(based on data from ROS2) and change color according with goal_status
            old(CALL, robot_phase::IDLE, [&](robot_phase ph)
                {
                std::string rname = ROBOT_PREFIX + std::to_string(node.uid); // name is obtained from node ID
                node.storage(node_external_name{}) = rname;

                std::lock_guard lgr(RobotStatesMutex);
                // if robot is in the map
                if (RobotStatesMap.find(rname) != RobotStatesMap.end()) {
                    // for each status of the robot
                    for (RobotStatus rs : RobotStatesMap[rname]) {
                        // print out the robot status
                        std::cout << "Robot " << rname << " status: "
                                  << "pos_x: " << rs.pos_x << ", "
                                  << "pos_y: " << rs.pos_y << ", "
                                  << "pos_z: " << rs.pos_z << ", "
                                  << "orient_w: " << rs.orient_w << ", "
                                  << "battery_percent_charge: " << rs.battery_percent_charge << ", "
                                  << "goal_status: " << rs.goal_status << ", "
                                  << "goal_code: " << rs.goal_code << std::endl;
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
                            // print that the robot is running
                            // std::cout << "Robot " << rname << " is in running" << endl;
                            manage_running_goal_status(CALL);
                        }
                        // if new state is REACHED:
                        else if (feedback::GoalStatus::REACHED == rs.goal_status) {
                            // std::cout << "Robot " << rname << " is in reached" << endl;
                            manage_reached_goal_status(CALL);
                        }
                        // if new state is UNKNOWN: new color is idle    
                        else if (feedback::GoalStatus::UNKNOWN == rs.goal_status) {
                            // std::cout << "Robot " << rname << " is in unknown" << endl;
                            manage_unknown_goal_status(CALL);
                        }
                        // if new state is FAILED: new color is failed        
                        else if (feedback::GoalStatus::FAILED == rs.goal_status) {
                            // std::cout << "Robot " << rname << " is in failed" << endl;
                            manage_failed_goal_status(CALL);
                        }
                        // if new state is ABORTED: new color is aborted        
                        else if (feedback::GoalStatus::ABORTED == rs.goal_status) {
                            // std::cout << "Robot " << rname << " is in aborted" << endl;
                            manage_aborted_goal_status(CALL);
                        }
                        // if new state is NO GOAL: new color is idle        
                        else if (feedback::GoalStatus::NO_GOAL == rs.goal_status) {
                            // std::cout << "Robot " << rname << " is in no goal" << endl;
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

                return ph; });
        }

        // PROCESS MANAGEMENT

        //! @brief Spawn process from goal list acquired, which you get from the feedback.
        FUN spawn_result_type spawn_process(ARGS, ::vector<goal_tuple_type> &NewGoalsList, int n_round)
        {
            // process new goals, emptying NewGoalsList
            return coordination::spawn(CALL, [&](goal_tuple_type const &goal)
                                       {
                status st = status::internal_output;

                field<int> test_nbr = fcpp::coordination::nbr(CALL, n_round);
                if (AP_ENGINE_DEBUG) {
                    std::cout << "test_nbr_in_process: " << test_nbr << std::endl;
                }

                // std::cout << "I am robot " << node.uid << " and my master is " << node.storage(scout_curr_worker{}) << std::endl;

                // if (isWorker(CALL))
                // {
                //     // print the goal_code and goal_action and which round it is
                //     std::cout << "Goal code: " << get<goal_code>(goal) << " Goal action: " << get<goal_action>(goal) << " Round: " << n_round << endl;
                //     std::cout << endl;
                // }
                node.storage(node_active{}) = 1;

                // assign index to the scout
                assignScout(CALL);

                // correct the indexes of scouts
                correctIndexes(CALL);
                

                // ACTION: ABORT GOAL
                // the two lines of check for process_goal and external_goal are because, we have simulation in AP and in gazebo(feedback)
                if (ABORT_ACTION == get<goal_action>(goal) &&
                    node.storage(node_process_goal{}) == get<goal_code>(goal) &&
                    node.storage(node_external_goal{}) == get<goal_code>(goal)) {
                    // std::cout << "Robot " << node.uid << " is in abort action" << endl;

                    manage_action_abort(CALL, goal, &st);
                }

                // ACTION: REACH GOAL
                else if (GOAL_ACTION == get<goal_action>(goal) && isActive(CALL)) {
                    // std::cout << "Robot " << node.uid << " is in goal action" << endl;
                    // print the goal_action and goal_action
                    // std::cout << "Goal action: " << get<goal_action>(goal) << " Goal code: " << get<goal_code>(goal) << " for robot " << node.uid << endl;

                    manage_action_goal(CALL, goal, &st, n_round);
                }

                termination_logic(CALL, st, goal);

                // Landing if status == border and pos_z > 0 
                if (!isWorker(CALL) && st == status::border && counter(CALL) == 1) {
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
            return make_tuple(node.current_time(), st); }, NewGoalsList);
        }

        //! @brief Manage termination of the spawn processes.
        FUN void manage_termination(ARGS, spawn_result_type &r)
        {
            // if process was terminating and now it's terminated, we have to change state machine to IDLE
            if (node.storage(node_process_status{}) == ProcessingStatus::TERMINATING)
            {
                // if process has been terminated, it isn't in the result map of spawn
                bool process_found = false;
                for (auto const &x : r)
                {
                    auto goal = x.first;
                    if (get<goal_code>(goal) == node.storage(node_process_goal{}))
                    {
                        process_found = true;
                    }
                }
                if (!process_found)
                {
                    std::cout << "Process with code " << node.storage(node_process_goal{}) << " not found, so move robot to IDLE" << endl;
                    node.storage(node_process_status{}) = ProcessingStatus::IDLE;
                }
            }

            // search on results if the computing processes has been terminated:
            //  - if it's terminated (or in other words, if the goal it's not in the "r" variable), we delete it from the map in the storage
            std::vector<std::string> goals_to_remove = {};
            for (auto const &x : node.storage(node_process_computing_goals{}))
            {
                auto goal = x.second;
                if (r.find(goal) == r.end())
                {
                    std::cout << "Remove process with code " << get<goal_code>(goal) << endl;
                    goals_to_remove.push_back(get<goal_code>(goal));
                }
            }
            for (auto const &gc : goals_to_remove)
            {
                remove_goal_from_computing_map(CALL, gc);
            }
        }

        //! @brief Main case study function.
        MAIN()
        {
            // for testing purposes
            int nubmer_of_masters = 2; // TODO: this should be passed in from a user
            // INITIALIZE VARS
            std::vector<goal_tuple_type> NewGoalsList{};
            int n_round = fcpp::coordination::counter(CALL);
            node.storage(node_countRound{}) = counter(CALL); // TODO: use this instead of n_round

            if (!node.storage(node_set{}))
                init_main_fn(CALL, n_round, nubmer_of_masters);

            // UPDATE DATA
            acquire_new_goals(CALL, NewGoalsList);

            // multi_worker_doesn't have this, since it is applied directly in the AP itself, and it doesn't have ROS2
            apply_feedback_to_ap(CALL);

            // call updateWorker so that the scouts get the correct worker. TODO: MOVE TO THE PROCESS. FOR NOW DECIDED TO NOT MOVE IT
            updateWorker(CALL);

            // update the count of scouts
            split(CALL, (isWorker(CALL)) ? node.uid : node.storage(scout_curr_worker{}), [&]() {return updateFollowersCount(CALL);}); // split according to the uid if current node is worker, otherwise use the worker of the current scout

            // Init Flocking
            // initialization(CALL); // I think we will not need this after the refactoring

            // Assign index to the scout, TODO: MOVE TO THE PROCESS
            // assignScout(CALL);

            // correctIndexes(CALL);

            // TODO: HERE IS WHERE I STOPPED. NOW THE INDEXES SHOULD BE CORRECT, THE SLAVES SHOULD BELONG TO CORRECT WORKERS
            // NOW THE GOAL NEEDS TO BE FOLLOWED(WORKER MOVED ACCORDING TO WHAT THE GOAL FROM ROS2 IS AND SLAVES SHOULD POSITION
            // ACCORDING TO ITS WORKER)

            // PROCESS MANAGEMENT
            spawn_result_type r = spawn_process(CALL, NewGoalsList, n_round);

            manage_termination(CALL, r);
        }

        //! @brief Export types used by the *_connection functions.
        FUN_EXPORT any_connection_t = export_list<
            int,
            bool,
            goal_tuple_type,
            robot_phase,
            process_tuple_type,
            infoWorkerType>;

        // TODO: refactor (cancellare le funzioni da togliere)
        FUN_EXPORT flocking_t = export_list<
            double,
            int,
            tuple<int, vec<3>>,
            vec<3>,
            tuple<bool, vec<3>>,
            tuple<int, int>,
            tuple<bool, double>,
            bool,
            tuple<bool, bool>,
            tuple<vec<3>, int>>;

        //! @brief Export types used by the main function (update it when expanding the program).
        struct main_t : public export_list<
                            any_connection_t,
                            flocking_t,
                            spawn_t<goal_tuple_type, status>,
                            diameter_election_t<tuple<real_t, device_t>>>

        {
        };

    }
}

#endif // NODES_AP_ENGINE_H_
