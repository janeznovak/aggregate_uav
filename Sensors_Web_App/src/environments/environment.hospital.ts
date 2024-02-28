/**
 * @license
 * Copyright Akveo. All Rights Reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 */
// The file contents for the current environment will overwrite these during build.
// The build system defaults to the dev environment which uses `environment.ts`, but if you do
// `ng build --env=prod` then `environment.prod.ts` will be used instead.
// The list of which env maps to which file can be found in `.angular-cli.json`.

// Use Partial<{ [key: string]: any }> to diff configuration from commonEnv
const env: Partial<typeof commonEnv> = {
  production: false,
  robots: 4,
  wearables: 3,
  robot_dashboard_show: true,
  wearable_dashboard_show: true,
  robot_dashboard_find_your_book_show: false,
  robot_dashboard_goal_table_show: false,
  robot_dashboard_robot_map_show: false,
  robot_dashboard_charts_show: true,
  title: "PoC CommonWears",
};

import { commonEnv } from "./environment.common";
export const environment = { ...commonEnv, ...env };