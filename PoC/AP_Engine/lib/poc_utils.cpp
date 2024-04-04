// Copyright © 2023 Gianluca Torta, Daniele Bortoluzzi. All Rights Reserved.

#include "lib/poc_utils.hpp"

using namespace std;


std::string get_env_var(const char* env_key, const std::string& default_value) {
    const char* env_value = std::getenv(env_key);
    if (env_value != nullptr) {
        std::string env_value_str(env_value);
        if (!env_value_str.empty() && env_value_str != "\n") {
            return env_value_str;
        }
    }
    return default_value;
}

std::vector<std::string> generate_robot_names(const std::string& robot_prefix, const int start, const int robots_count) {
    std::vector<std::string> prefixes;

    for (int i = start; i < start+robots_count; ++i) {
        prefixes.push_back(robot_prefix + std::to_string(i));
    }

    return prefixes;
}

std::string read_string_env(const std::string& env_key, const std::string& default_str) {
    return get_env_var(env_key.c_str(), default_str);
}

int read_int_env(const std::string& env_key, const int default_int) {
    std::string env_str = get_env_var(env_key.c_str(), std::to_string(default_int));
    int env_int = stoi(env_str);
    return env_int; 
}

double read_double_env(const std::string& env_key, const double default_double) {
    std::string env_str = get_env_var(env_key.c_str(), std::to_string(default_double));
    double env_double = stod(env_str);
    return env_double; 
}

// TODO: use a map instead of a vector
std::string  get_robot_name(std::vector<std::string> robots, int node_uid) {
    std::string robot_name = "";
    return robots[node_uid]; // convert node.uid to index of array
    return robot_name;
}