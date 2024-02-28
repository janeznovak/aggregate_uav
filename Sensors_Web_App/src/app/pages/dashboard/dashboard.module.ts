import { NgModule } from '@angular/core';
import {
  NbActionsModule,
  NbButtonModule,
  NbCardModule,
  NbTabsetModule,
  NbUserModule,
  NbRadioModule,
  NbCheckboxModule,
  NbSelectModule,
  NbListModule,
  NbIconModule,
  NbStepperModule,
  NbProgressBarModule
} from '@nebular/theme';
import { NgxEchartsModule } from 'ngx-echarts';
import { ThemeModule } from '../../@theme/theme.module';
// tables
import { GoalTableComponent } from './common/goal-table/goal-table.component';
// canvas
import { RobotMapComponent } from './robot-map/robot-map.component';

import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { RobotDashboardComponent } from './robot-dashboard/robot-dashboard.component';
import { RobotStatusCardComponent } from './robot-status-card/robot-status-card.component';
import { WearableStatusCardComponent } from './wearable-status-card/wearable-status-card.component';
// import { ContactsComponent } from './contacts/contacts.component';
// import { RoomsComponent } from './rooms/rooms.component';
// import { RoomSelectorComponent } from './rooms/room-selector/room-selector.component';
// import { TemperatureComponent } from './temperature/temperature.component';
// import { TemperatureDraggerComponent } from './temperature/temperature-dragger/temperature-dragger.component';
// import { KittenComponent } from './kitten/kitten.component';
// import { SecurityCamerasComponent } from './security-cameras/security-cameras.component';
// import { ElectricityComponent } from './electricity/electricity.component';
// import { ElectricityChartComponent } from './electricity/electricity-chart/electricity-chart.component';
// import { WeatherComponent } from './weather/weather.component';
// import { SolarComponent } from './solar/solar.component';
// import { PlayerComponent } from './rooms/player/player.component';
// import { TrafficComponent } from './traffic/traffic.component';
// import { TrafficChartComponent } from './traffic/traffic-chart.component';

// tables
import {  NbInputModule, NbTreeGridModule } from '@nebular/theme';
import { Ng2SmartTableModule } from 'ng2-smart-table';

// feedback canvas
import { NgxCanvasModule } from 'ngx-canvas';

// goal creation
import { GoalCreationComponent } from './goal-creation/goal-creation.component';
import { GoalsByNodeChartComponent } from './common/goals-by-node-chart/goals-by-node-chart.component';

// wearable creation
import { WearableSOSCreationComponent } from './wearable-sos-creation/wearable-sos-creation.component';

import { ChartModule } from 'angular2-chartjs';
import { NgxChartsModule } from '@swimlane/ngx-charts';
import { GoalsByStatusChartComponent } from './common/goals-by-status-chart/goals-by-status-chart.component';
import { WearableDashboardComponent } from './wearable-dashboard/wearable-dashboard.component';

@NgModule({
  imports: [
    // tables
    NbInputModule,
    Ng2SmartTableModule,

    // canvas
    NgxCanvasModule,
    NbCheckboxModule,

    // goal creation
    NbStepperModule,
    ReactiveFormsModule,

    // charts
    NgxChartsModule,
    ChartModule,

    // battery
    NbProgressBarModule,

    FormsModule,
    ThemeModule,
    NbCardModule,
    NbUserModule,
    NbButtonModule,
    NbTabsetModule,
    NbActionsModule,
    NbRadioModule,
    NbSelectModule,
    NbListModule,
    NbIconModule,
    NbButtonModule,
    NgxEchartsModule,
  ],
  declarations: [
    // tables
    GoalTableComponent,
    RobotDashboardComponent,
    WearableDashboardComponent,
    WearableStatusCardComponent,
    RobotStatusCardComponent,
    // TemperatureDraggerComponent,
    // ContactsComponent,
    // RoomSelectorComponent,
    // TemperatureComponent,
    // RoomsComponent,
    // KittenComponent,
    // SecurityCamerasComponent,
    // ElectricityComponent,
    // ElectricityChartComponent,
    // WeatherComponent,
    // PlayerComponent,
    // SolarComponent,
    // TrafficComponent,
    // TrafficChartComponent,
    RobotMapComponent,
    GoalCreationComponent,
    GoalsByNodeChartComponent,
    GoalsByStatusChartComponent,
    WearableSOSCreationComponent
  ],
  exports: [
  ]
})
export class DashboardModule { }
