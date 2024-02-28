/**
 * @license
 * Copyright Akveo. All Rights Reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 */
// The file contents for the current environment will overwrite these during build.
// The build system defaults to the dev environment which uses `environment.ts`, but if you do
// `ng build --env=prod` then `environment.prod.ts` will be used instead.
// The list of which env maps to which file can be found in `.angular-cli.json`.

export const commonEnv = {
  production: false,
  robots: 5,
  wearables: 3,
  robots_color: ["#8f00ff", "#3CB371", "#C71585", "#1f3a3d", "#0000FF"],
  items: 8,
  axis_x_length: 4.90,
  axis_y_length: 7,
  base_path_http_server: "http://localhost:8080/",
  api_node_id_placehoder: "#NODEID",
  api_wearable_id_placehoder: "#WEARABLEID",
  api_goals: "goals",
  api_wearable_sos: "wearable-sos",
  api_feedbacks_last_by_node: "feedbacks/node/#NODEID/last",
  api_measurements_last_by_node: "measurements/node/#NODEID/last",
  api_stats_total_goals_by_node: "stats/total-goals-by-node",
  api_stats_total_goals_by_status: "stats/total-goals-by-status",
  api_wearable_feedbacks_last_by_wearable: "wearable-feedbacks/wearable/#WEARABLEID/last",
  robot_dashboard_show: true,
  wearable_dashboard_show: true,
  robot_dashboard_find_your_book_show: true,
  robot_dashboard_goal_table_show: true,
  robot_dashboard_robot_map_show: true,
  robot_dashboard_charts_show: true,
  title: "PoC",
};