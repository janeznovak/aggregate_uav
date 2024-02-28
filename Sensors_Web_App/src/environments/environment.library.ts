/**
 * @license
 * Copyright Akveo. All Rights Reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 */
// The file contents for the current environment will overwrite these during build.
// The build system defaults to the dev environment which uses `environment.ts`, but if you do
// `ng build --env=prod` then `environment.prod.ts` will be used instead.
// The list of which env maps to which file can be found in `.angular-cli.json`.

import { commonEnv } from "./environment.common";

// Use Partial<{ [key: string]: any }> to diff configuration from commonEnv
const env: Partial<typeof commonEnv> = {
  production: false,
  robots: 5,
  wearables: 0,
  robots_color: ["#8f00ff", "#3CB371", "#C71585", "#1f3a3d", "#0000FF"],
  items: 8,
  axis_x_length: 4.90,
  axis_y_length: 7,
  robot_dashboard_show: true,
  wearable_dashboard_show: false,
  robot_dashboard_find_your_book_show: true,
  robot_dashboard_goal_table_show: true,
  robot_dashboard_robot_map_show: false,
  robot_dashboard_charts_show: true,
  title: "PoC Robobase",
};


// Export all settings of common replaced by dev options
export const environment = { ...commonEnv, ...env };