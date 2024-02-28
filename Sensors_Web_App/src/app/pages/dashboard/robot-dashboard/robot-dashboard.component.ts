import {Component, OnDestroy} from '@angular/core';
import { NbThemeService } from '@nebular/theme';
import { takeWhile } from 'rxjs/operators' ;
import { FeedbackService } from '../../../@core/services/feedback.service';
import { MeasurementService } from '../../../@core/services/measurement.service';
import { StatusIconRendererComponent } from '../common/goal-table/status-icon-render.component';
import { SolarData } from '../../../@core/data/solar';
import { environment } from "../../../../environments/environment";
import { Observable } from 'rxjs';
import { Feedback } from '../../../@core/data/feedback';
import { getRobotName } from '../../../@core/utils/poc-utils'
import { Measurement } from '../../../@core/data/measurement';
import { getGoalStatusDescription } from '../../../@core/utils/poc-utils'
import { getGoalStatusIcon } from '../../../@core/utils/poc-utils'

interface RobotCardSettings {
  title: string;
  iconClass: string;
  type: string;
  on: boolean;
  bar: number;
  lastUpdate: Date;
  temperature: number;
  humidity: number;
  pressure: number;
  feedback_obs: Observable<Feedback>
  measurement_obs: Observable<Measurement>
}

@Component({
  selector: 'ngx-robot-dashboard',
  styleUrls: ['./robot-dashboard.component.scss'],
  templateUrl: './robot-dashboard.component.html',
})
// TODO: move status management to another component
export class RobotDashboardComponent implements OnDestroy {

  private alive = true;

  timerStatuses;

  solarValue: number;

  statusCards: string;

  statusCardsSet: RobotCardSettings[] = [];

  statusCardsByThemes: {
    default: RobotCardSettings[]
  } = {
    default: this.statusCardsSet
  };

  goal_creation_show = environment.robot_dashboard_find_your_book_show;
  goal_table_show = environment.robot_dashboard_goal_table_show;
  goal_map_show = environment.robot_dashboard_robot_map_show;
  goal_charts_show = environment.robot_dashboard_charts_show;

  goalActions: string[] = ['GOAL', 'ABORT', 'DOCK', 'UNDOCK'];

  customTableElemMapFunction = (el) => {
    el.statusDesc = getGoalStatusDescription(el.status)
    el.statusIcon = getGoalStatusIcon(el.status)
    if (el.nodeId)
      el.nodeName = getRobotName(el.nodeId)
    return el;
  };

  columns: any = {
    goalCode: {
      title: 'Codice Goal',
      type: 'string',
    },
    referenceCode: {
      title: 'Elemento cercato',
      type: 'string',
    },
    statusDesc: {
      title: 'Stato',
      type: 'string',
    },
    statusIcon: {
      title: 'Icona stato',
      type: 'custom',
      renderComponent: StatusIconRendererComponent,
      filter: false
    },
    // nodeId: {
    //   title: 'ID robot',
    //   type: 'number',
    // },
    nodeName: {
      title: 'Robot',
      type: 'string',
    },
    // timestampCreation: {
    //   title: 'Creazione',
    //   type: 'date',
    // },
    timestampUpdate: {
      title: 'Aggiornamento',
      type: 'date',
    },
  };

  updateRobotStatuses() {
    for (let index = 0; index < this.statusCardsSet.length; index++) {
      const element = this.statusCardsSet[index];

      let subscription_feedback =
        element.feedback_obs
        .subscribe((feedback) => {
          if (feedback) {
            element.type = (feedback.batteryPercentCharge) <= 0 ? 'warning' : 'info'
            element.on = (feedback.batteryPercentCharge) <= 0 ? false : true
            element.bar = Math.trunc(feedback.batteryPercentCharge)
            element.lastUpdate = new Date(feedback.timestamp)
            subscription_feedback.unsubscribe();
          }
        });

        let subscription_measurement =
          element.measurement_obs
          .subscribe((measurement) => {
            if (measurement) {
              element.temperature = (measurement.measurementData.temperature) ?  measurement.measurementData.temperature : undefined
              element.humidity = (measurement.measurementData.humidity) ? measurement.measurementData.humidity : undefined
              element.pressure = (measurement.measurementData.pressure) ? measurement.measurementData.pressure : undefined
              subscription_measurement.unsubscribe();
            }
          });

    }
  }

  constructor(private themeService: NbThemeService,
              private solarService: SolarData,
              private feedbackService: FeedbackService,
              private measurementService: MeasurementService) {
    this.themeService.getJsTheme()
      .pipe(takeWhile(() => this.alive))
      .subscribe(theme => {
        this.statusCards = this.statusCardsByThemes[theme.name];
    });

    for (let index = 0; index < environment.robots; index++) {
      const nodeId = index+1;

      let feedback_obs = this.feedbackService.getLastFeedbackByNodeId(nodeId)
      let measurement_obs = this.measurementService.getLastMeasurementByNodeId(nodeId)

      this.statusCardsSet.push(
        {
          title: getRobotName(nodeId),
          iconClass: 'nb-gear',
          type: 'warning',
          feedback_obs: feedback_obs,
          measurement_obs: measurement_obs,
          on: false,
          bar: 0.0,
          lastUpdate: undefined,
          temperature: undefined,
          humidity: undefined,
          pressure: undefined
        }
      );
    }

    this.timerStatuses = setInterval(() => {
      this.updateRobotStatuses();
    }, 2000);
  }

  ngOnDestroy() {
    this.alive = false;
    if (this.timerStatuses) {
      clearInterval(this.timerStatuses);
    }
  }
}
