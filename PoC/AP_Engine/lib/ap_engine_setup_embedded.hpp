
// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

/**
 * @file ap_engine_setup_embedded.hpp
 * @brief Setup for the AP engine for the NODES demo in embedded environment.
 */

#ifndef NODES_AP_ENGINE_SETUP_EMBEDDED_H_
#define NODES_AP_ENGINE_SETUP_EMBEDDED_H_

#include <cmath>
#include "lib/poc_config.hpp"
#include "lib/fcpp.hpp"
#include "lib/drivers/wifi/driver.hpp"

// TODO: are useful?
#define DEGREE       10  // maximum degree allowed for a deployment
#define DIAMETER     10  // maximum diameter in hops for a deployment
#define WINDOW_TIME  60  // time in seconds during which positive node information is retained
#define PRESS_TIME   5   // time in seconds of button press after which termination is triggered
#define BUFFER_SIZE  40  // size in KB to be used for buffering the output

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @cond INTERNAL
namespace coordination {
    struct main;   // forward declaration of main function
    struct main_t; // forward declaration of main exports
}
//! @endcond

/**
 * @brief Create custom_deployment component for embedded environment
 */
namespace component {
    DECLARE_COMBINE(custom_deployment, hardware_logger, simulated_positioner, persister, storage, hardware_connector, timer, scheduler, hardware_identifier, randomizer, calculus);
}

namespace coordination {
    namespace tags {
    //! @brief Total round count since start.
    struct round_count {};
    //! @brief A shared global clock.
    struct global_clock {};
    //! @brief Minimum UID in the network.
    struct min_uid {};
    //! @brief Distance in hops to the device with minimum UID.
    struct hop_dist {};
    //! @brief Maximum stack size ever experienced.
    struct max_stack {};
    //! @brief Maximum heap size ever experienced.
    struct max_heap {};
    //! @brief Maximum message size ever experienced.
    struct max_msg {};
    //! @brief Percentage of transmission success for the strongest link.
    struct strongest_link {};
    //! @brief The degree of the node.
    struct degree {};
    //! @brief List of neighbours encountered at least 50% of the times.
    struct nbr_list {};
} // tags

}

//! @brief Namespace for component options.
namespace option {

//! @brief Import tags to be used for component options.
using namespace component::tags;
//! @brief Import tags used by aggregate functions.
using namespace coordination::tags;

//! @brief Dictates that rounds are happening (first event, other events).
using schedule_type = round_schedule<sequence::periodic_i<tavg, tavg>>;

//! @brief Shorthand for a constant numeric distribution.
template <intmax_t num, intmax_t den = 1>
using n = distribution::constant_n<double, num, den>;

//! @brief Shorthand for an constant input distribution.
template <typename T, typename R = std::unordered_map<std::string, std::vector<device_t>>>
using subcode_map_distr = distribution::constant_i<R, T>;

//! @brief The general simulation options.
DECLARE_OPTIONS(list_embedded,
    parallel<true>,     // yes multithreading on node rounds and communication
    program<coordination::main>,   // program to be run (refers to MAIN in ap_engine.hpp)
    exports<coordination::main_t>, // export type list (types used in messages)
    //    log_schedule<sequence::periodic_n<1, 0, 1>>, // the sequence generator for log events on the network
    retain_type,
    schedule_type,
    dimension<fcpp::coordination::dim>, // dimensionality of the space
    // the basic contents of the node storage
    tuple_store<
        seed,                               uint_fast32_t,
        speed,                              double,
        devices,                            size_t,
        sidex,                              real_t,
        sidey,                              real_t,
        node_color,                         color,
        left_color,                         color,
        right_color,                        color,
        node_size,                          double,
        node_shape,                         shape,
        node_label_size,                    double,
        node_label_text,                    string,
        node_label_color,                   color,
        node_shadow_shape,                  shape,
        node_shadow_size,                   double,
        node_shadow_color,                  color,
        node_external_status,               feedback::GoalStatus,
        node_external_status_update_time,   std::time_t,
        node_external_goal,                 string,
        node_external_name,                 string,
        node_process_status,                ProcessingStatus,
        node_process_goal,                  string,
        node_process_computing_goals,       std::unordered_map<std::string, fcpp::option::data::goal_tuple_type>,
        node_battery_charge,                double,
        node_offset_pos_x,                  real_t,
        node_offset_pos_y,                  real_t,
        tavg,                               real_t,
        nodes_by_goal_subcode,              std::unordered_map<std::string, std::vector<device_t>>,
        // from fcpp-miosix
        round_count,                        uint16_t,
        global_clock,                       times_t,
        min_uid,                            device_t,
        hop_dist,                           hops_t,
        max_stack,                          uint16_t,
        max_heap,                           uint32_t,
        max_msg,                            uint8_t,
        strongest_link,                     int8_t,
        degree,                             int8_t,
        nbr_list,                           std::vector<device_t>
    >,
        // data initialisation
    init<
        node_offset_pos_x,                  n<0,1>, // -> 0
        node_offset_pos_y,                  n<0,1>, // -> 0
        tavg,                               i<tavg>, // seconds of mean period
        nodes_by_goal_subcode,              subcode_map_distr<nodes_by_goal_subcode>
    >
);


}


}

#endif // NODES_AP_ENGINE_SETUP_EMBEDDED_H_
