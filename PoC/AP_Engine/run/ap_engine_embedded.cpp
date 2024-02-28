// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

/**
 * @file ap_engine_embedded.cpp
 * @brief Runs the AP engine for the NODES demo in embedded environment.
 */

#define RUN_EMBEDDED

#include "lib/poc_config.hpp"
#include "lib/ap_engine.hpp"
#include "lib/ap_engine_setup_embedded.hpp"

#include <chrono>
#include <thread>
#include "lib/poc_file_watcher.hpp"

using namespace fcpp;
using namespace component::tags;
using namespace common;

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Namespace for component options.
namespace option {

//! @brief Import tags to be used for component options.
using namespace component::tags;
//! @brief Import tags used by aggregate functions.
using namespace coordination::tags;
//! @brief Import components for DECLARE
using namespace component;

//! @brief Main FCPP option setup.
DECLARE_OPTIONS(custom_deployment,
    fcpp::option::list_embedded
);

}

}

int main() {
    // file watcher
    poc_file_watcher poc_file_watcher;
    poc_file_watcher.run_watch_files();

    std::cout << "COMPUTED graph_diameter:" << fcpp::coordination::graph_diameter << endl;

    std::unordered_map<std::string, std::vector<device_t>> nodes_by_goal_subcode = {};

    // Type for the network object.
    using net_t = fcpp::component::custom_deployment<fcpp::option::custom_deployment>::net;
    // The initialisation values (simulation name, ...).
    auto init_v = common::make_tagged_tuple
    <
        fcpp::option::x, 
        fcpp::option::hoodsize, 
        fcpp::coordination::tags::tavg, 
        fcpp::coordination::nodes_by_goal_subcode
    >(
        make_vec(0, 0, 0),
        device_t{DEGREE},
        ROUND_PERIOD,
        nodes_by_goal_subcode
    );
    // Construct the network object.
    net_t network{init_v};
    // Run the program until exit.
    network.run();
    
    return 0;
}

