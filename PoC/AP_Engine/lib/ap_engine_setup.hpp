// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

/**
 * @file ap_engine_setup.hpp
 * @brief Setup for the AP engine.
 */

#ifndef NODES_AP_ENGINE_SETUP_H_
#define NODES_AP_ENGINE_SETUP_H_

#include <cmath>
#include "lib/poc_config.hpp"
#include "lib/fcpp.hpp"

#define DEFAULT 0
#define HIGHDISTANCE 1
#define LOWDISTANCE 2

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp
{

    //! @brief Dummy ordering between positions (allows positions to be used as secondary keys in ordered tuples).
    // TODO: spostarla in namespace fcpp
    template <size_t n>
    bool operator<(vec<n> const &, vec<n> const &)
    {
        return false;
    }

    //! @cond INTERNAL
    namespace coordination
    {
        struct main;   // forward declaration of main function
        struct main_t; // forward declaration of main exports
    }
    //! @endcond

    namespace coordination
    {
        namespace tags
        {
            //! @brief The average round interval of a device.
            struct tavg
            {
            };

            //! @brief The variance of round timing in the network.
            struct tvar
            {
            };

            //! @brief The number of devices.
            struct devices
            {
            };

            //! @brief The movement speed of devices.
            struct speed
            {
            };

            //! @brief The horizontal side of deployment area.
            struct sidex
            {
            };

            //! @brief The vertical side of deployment area.
            struct sidey
            {
            };

            //! @brief Color of the current node.
            struct node_color
            {
            };

            //! @brief Left color of the current node.
            struct left_color
            {
            };

            //! @brief Right color of the current node.
            struct right_color
            {
            };

            //! @brief Size of the current node.
            struct node_size
            {
            };

            //! @brief Shape of the current node.
            struct node_shape
            {
            };

            //! @brief Size of the label of the current node.
            struct node_label_size
            {
            };

            //! @brief Text of the label of the current node.
            struct node_label_text
            {
            };

            //! @brief Shape of the shadow of current node.
            struct node_shadow_shape
            {
            };

            //! @brief Size of the shadow of the current node.
            struct node_shadow_size
            {
            };

            //! @brief Color of the shadow of the current node.
            struct node_shadow_color
            {
            };

            //! @brief Status of the current node read by robot feedback.
            struct node_external_status
            {
            };

            //! @brief Timestamp of the last change of the status goal read by robot feedback.
            struct node_external_status_update_time
            {
            };

            //! @brief Goal of the current node read by robot feedback.
            struct node_external_goal
            {
            };

            //! @brief External name of a node used by applications (different from AP).
            struct node_external_name
            {
            };

            //! @brief Status of the AP process of the current node.
            struct node_process_status
            {
            };

            //! @brief Goal of the current node in AP process.
            struct node_process_goal
            {
            };

            //! @brief List of computing goals of the current node in AP process.
            struct node_process_computing_goals
            {
            };

            //! @brief Battery charge of the current node.
            struct node_battery_charge
            {
            };

            //! @brief Offset of "x" position of the node
            struct node_offset_pos_x
            {
            };

            //! @brief Offset of "y" position of the node
            struct node_offset_pos_y
            {
            };

            //! @brief Map with nodes enabled for specific goal.subcode
            struct nodes_by_goal_subcode
            {
            };

            //! @brief Action of the goal
            struct goal_action
            {
            };

            //! @brief Code of the goal
            struct goal_code
            {
            };

            //! @brief Position X of the goal
            struct goal_pos_x
            {
            };

            //! @brief Position Y of the goal
            struct goal_pos_y
            {
            };

            //! @brief Position Z of the goal
            struct goal_pos_z
            {
            };

            //! @brief Orient W of the goal
            struct goal_orient_w
            {
            };

            //! @brief Source of the goal
            struct goal_source
            {
            };

            //! @brief Priority of the goal
            struct goal_priority
            {
            };

            //! @brief Subcode of the goal
            struct goal_subcode
            {
            };

            // Start Flocking
            struct node_posMaster
            {
            };
            struct node_numberOfSlave
            {
            };
            struct node_indexSlave
            {
            };
            struct node_myRadiant
            {
            };
            struct node_vecMyRadiant
            {
            };
            struct node_vecMyVersor
            {
            };
            struct node_fixIndex
            {
            };
            struct node_maxNumberOfSlave
            {
            };
            struct node_flagDistance
            {
            };
            struct position_error
            {
            };
            struct node_startPosition
            {
            };
            // END

            //! @brief Is the node initialised yet
            struct node_set
            {
            };
            //! @brief Is the node a worker
            struct node_isWorker
            {
            };
            //! @brief Number of the round
            struct node_countRound
            {
            };

            struct required_scouts
            {
            };

            struct scout_need
            {
            };
            struct expected_dist_worker_scout
            {
            };
            struct scout_isCharging
            {
            };
            struct node_active
            {
            };
            struct was_running_last_round
            {
            };
            struct goal_start_time
            {
            };

            //! @brief tags for infoWorker.
            struct infoW_active
            {
            };
            struct infoW_need
            {
            };
            struct infoW_nodeDistance
            {
            };
            struct infoW_position
            {
            };
            struct infoW_velocity
            {
            };
            struct infoW_nodeId
            {
            };

            struct scout_curr_worker // uid of the worker the scout is currently assigned to. -1 if not assigned
            {
            };
        } // tags

#ifndef AP_USE_CASE
#define AP_USE_CASE DEFAULT
#endif

#if AP_USE_CASE == HIGHDISTANCE
        //! @brief Communication radius.
        constexpr size_t comm = 10;
#elif AP_USE_CASE == LOWDISTANCE
        //! @brief Communication radius.
        constexpr size_t comm = AP_COMM_RANGE;
#else
        //! @brief Communication radius.
        constexpr size_t comm = AP_COMM_RANGE;
#endif

        //! @brief Retain time for messages.
        constexpr size_t retain = AP_RETAIN_SEC;

        //! @brief Dimensionality of the space.
        constexpr size_t dim = 3;

        //! @brief Min area of the rectangle.
        const vec<3> amin = make_vec(0.0, 0.0, 0.0);

        //! @brief Max area of the rectangle.
        const vec<3> amax = make_vec(AXIS_X_LENGTH, AXIS_Y_LENGTH, 0.0);

        //! @brief Diameter of nodes graph.
        const int graph_diameter = (CUSTOM_GRAPH_DIAMETER != NULL_INT_VALUE)
                                       ? CUSTOM_GRAPH_DIAMETER
                                       // 2 * min(n_robots, 2 * (round of max_distance / communication range))
                                       : 2 * static_cast<int>(std::min(ROBOTS_COUNT * 1.0, 2.0 * ceil(norm(amax - amin) / comm)));

        // Start Flocking

        //! @brief Distance CircularCrown-slave, equivale al raggio della corona circolare della circonferenza,
        constexpr double distanceCircularCrown = (std::min(2.77, 3.29) * (comm - ((comm / 100) * 25)) / comm) * 0.25;

#if AP_USE_CASE == HIGHDISTANCE
        //! @brief Distance master-slave, equivale al raggio della circonferenza della formazione.
        constexpr double distanceMasterSlave = distanceCircularCrown - ((distanceCircularCrown / 100) * 50);

#elif AP_USE_CASE == LOWDISTANCE
        //! @brief Distance master-slave, equivalente al range di comunicazione
        constexpr double distanceMasterSlave = distanceCircularCrown - ((distanceCircularCrown / 100) * 80);

#else
        //! @brief Distance master-slave, equivale al raggio della circonferenza della formazione.
        constexpr double distanceMasterSlave = distanceCircularCrown - ((distanceCircularCrown / 100) * 50); // in other code called distanceWorkerScout
#endif

        //! @brief The maximum communication range between nodes; TODO: change this to test with different data, check which range is ok.
        constexpr size_t communication_range = 100; // TODO: test with different data, also test with different retain time

        //! @brief Constant minimum number of nodes to form a circle
        constexpr int minNodesToFormCircle = 5;
        //! @brief Number of people in the area.
        constexpr int node_num = minNodesToFormCircle + 5;
        //! @brief Constant pi-greco, usata per i calcoli trigonometrici.
        constexpr double pi = 3.14159265358979323846;
        //! @brief Minimum distance between devices to avoid colliding.
        constexpr double minDistance = ((2 * distanceMasterSlave * pi) / (node_num - 1)) * 0.5;

        //! @brief Hardness constant for elastic force Master-Slave.
        constexpr double hardnessMasterSlave = (distanceMasterSlave / 10000) * 2;
        //! @brief Hardness constant for elastic force CircularCrown-Slave.
        constexpr double hardnessCircularCrown = (distanceCircularCrown / 10000) * 2;
        //! @brief Hardness constant for elastic force Slave-Slave.
        constexpr double hardnessSlaveSlave = 0.01;
        //! @brief Percentage increment force if distance devices is < minimum distance, high collision risk
        constexpr double incrementForce = 50;
        // END Flocking

        // simulator vars
        constexpr vec<2> computing_colors = make_vec(0xADAD00FF, fcpp::YELLOW);

        constexpr fcpp::packed_color idle_color = BLACK;
        constexpr fcpp::packed_color running_color = ORANGE;
        constexpr fcpp::packed_color reached_goal_color = GREEN;
        constexpr fcpp::packed_color failed_goal_color = TOMATO;
        constexpr fcpp::packed_color aborted_goal_color = RED;
        constexpr fcpp::packed_color discharged_color = MAROON;

        constexpr int nWorker = 2;
        constexpr int nScout = 4;
        constexpr int nWorkerScout = nScout / nWorker;
    }

    //! @brief Namespace for component options.
    namespace option
    {


        //! @brief Dimensionality of the space.
        constexpr size_t dim = 3;

        //! @brief Import tags to be used for component options.
        using namespace component::tags;
        //! @brief Import tags used by aggregate functions.
        using namespace coordination::tags;

        //! @brief Maximum admissible value for a seed.
        constexpr size_t seed_max = std::min<uintmax_t>(std::numeric_limits<uint_fast32_t>::max(), std::numeric_limits<intmax_t>::max());

        //! @brief Shorthand for a constant numeric distribution.
        template <intmax_t num, intmax_t den = 1>
        using n = distribution::constant_n<double, num, den>;

        //! @brief Shorthand for a uniform numeric distribution.
        template <intmax_t max, intmax_t min = 0>
        using nu = distribution::interval_n<double, min, max>;

        //! @brief Shorthand for an constant input distribution.
        template <typename T, typename R = double>
        using i = distribution::constant_i<R, T>;

        //! @brief Shorthand for an constant input distribution.
        template <typename T, typename R = std::unordered_map<std::string, std::vector<device_t>>>
        using subcode_map_distr = distribution::constant_i<R, T>;

        //! @brief The randomised sequence of rounds for every node
        using round_s = sequence::periodic<
            distribution::interval_n<times_t, 0, 1, 10>,
            distribution::weibull_i<times_t, tavg, tvar>>;

        //! @brief The distribution of initial node positions (to be overwritten by initial object positions).
        using rectangle_d = distribution::rect<n<0>, n<0>, n<0>, i<sidex>, i<sidey>, n<0>>;

        //! @brief Dictates that messages are thrown away after N second. Remember to change value according to schedule_type.
        using retain_type = retain<metric::retain<fcpp::coordination::retain>>;

        using infoWorkerType = common::tagged_tuple_t<
            infoW_active, bool,
            infoW_need, int,
            infoW_nodeDistance, double,
            infoW_position, vec<3>,
            infoW_nodeId, int>;

        namespace data
        {
            //! @brief A representation of goal properties using tagged_tuple
            using goal_tuple_type = common::tagged_tuple_t<
                goal_action, string,
                goal_code, string,
                goal_pos_x, float,
                goal_pos_y, float,
                goal_pos_z, float,
                goal_orient_w, float,
                goal_source, string,
                goal_priority, int,
                goal_subcode, string>;

            //! @brief A representation of process properties using tagged_tuple
            using process_tuple_type = common::tagged_tuple_t<
                // TODO
                goal_code, string>;
        }

        //! @brief The general simulation options.
        DECLARE_OPTIONS(list,
                        parallel<false>,                                              // no multithreading on node rounds
                        synchronised<false>,                                          // optimise for asynchronous networks
                        program<coordination::main>,                                  // program to be run (refers to MAIN in process_management.hpp)
                        exports<coordination::main_t>,                                // export type list (types used in messages)
                        retain_type,                                                  // retain time for messages
                        round_schedule<round_s>,                                      // the sequence generator for round events on nodes
                        log_schedule<sequence::periodic_n<1, 0, 1>>,                  // the sequence generator for log events on the network
                        spawn_schedule<sequence::multiple<i<devices, size_t>, n<0>>>, // the sequence generator of node creation events on the network
                        // the basic contents of the node storage
                        tuple_store<
                            seed, uint_fast32_t,
                            speed, double,
                            devices, size_t,
                            sidex, real_t,
                            sidey, real_t,
                            node_color, color,
                            left_color, color,
                            right_color, color,
                            node_size, double,
                            node_shape, shape,
                            node_label_size, double,
                            node_label_text, string,
                            node_shadow_shape, shape,
                            node_shadow_size, double,
                            node_shadow_color, color,
                            node_external_status, feedback::GoalStatus,
                            node_external_status_update_time, std::time_t,
                            node_external_goal, string,
                            node_external_name, string,
                            node_process_status, ProcessingStatus,
                            node_process_goal, string,
                            node_process_computing_goals, std::unordered_map<std::string, fcpp::option::data::goal_tuple_type>,
                            node_battery_charge, double,
                            node_offset_pos_x, real_t,
                            node_offset_pos_y, real_t,
                            tavg, real_t,
                            tvar, real_t,
                            nodes_by_goal_subcode, std::unordered_map<std::string, std::vector<device_t>>,
                            node_set, bool,
                            node_isWorker, bool,
                            node_countRound, int,
                            required_scouts, int,
                            scout_need, int,
                            expected_dist_worker_scout, double,
                            scout_curr_worker, int,
                            scout_isCharging, bool,
                            node_active, int,
                            was_running_last_round, bool,
                            goal_start_time, std::chrono::time_point<std::chrono::steady_clock>,

                            // START Flocking
                            node_posMaster, tuple<bool, vec<3>>,
                            node_numberOfSlave, int,
                            node_indexSlave, tuple<int, int>,
                            node_myRadiant, double,
                            node_vecMyRadiant, vec<3>,
                            node_vecMyVersor, vec<3>,
                            node_fixIndex, bool,
                            node_maxNumberOfSlave, int,
                            node_flagDistance, bool,
                            position_error, double,
                            node_startPosition, vec<3> //! Initialized only for the nominal test
                            // END Flocking
                        >,
                        // data initialisation
                        init<
                            x, rectangle_d,
                            node_active, n<1>,
                            seed, functor::cast<distribution::interval_n<double, 0, seed_max>, uint_fast32_t>,
                            speed, functor::div<i<speed>, n<0>>,
                            devices, i<devices>,
                            tavg, i<tavg>, // seconds of mean period
                            tvar, i<tvar>, // seconds of variance period
                            node_offset_pos_x, i<node_offset_pos_x>,
                            node_offset_pos_y, i<node_offset_pos_y>,
                            nodes_by_goal_subcode, subcode_map_distr<nodes_by_goal_subcode>>,
                        dimension<fcpp::coordination::dim>,                                             // dimensionality of the space
                        connector<connect::fixed<fcpp::coordination::communication_range, 1, dim>>, // connection allowed within a radius comm range
                        // connector<connect::radial<80, connect::fixed<fcpp::coordination::comm, 1, dim>>>,                                                                                                // connector<connect::fixed<fcpp::coordination::comm, 1, fcpp::coordination::dim>>, // connection allowed within a fixed comm range
                        // connector<connect::radial<80, connect::fixed<fcpp::coordination::communication_range, 1, dim>>>,
                        shape_tag<node_shape>,                                                                                // the shape of a node is read from this tag in the store
                        size_tag<node_size>,                                                                                  // the size of a node is read from this tag in the store
                        color_tag<node_color, left_color, right_color>,                                                       // colors of a node are read from these
                        label_size_tag<node_label_size>,                                                                      // the size of the node label is read from this tag in the store
                        label_text_tag<node_label_text>,                                                                      // the text of the node label is read from this tag in the store
                        shadow_shape_tag<node_shadow_shape>,                                                                  // the shape of a shadow is read from this tag in the store
                        shadow_size_tag<node_shadow_size>,                                                                    // the size of the shadow is read from this tag in the store
                        shadow_color_tag<node_shadow_color>                                                                   // color of the node shape is read from these
        );

    }

}

#endif // NODES_AP_ENGINE_SETUP_H_
