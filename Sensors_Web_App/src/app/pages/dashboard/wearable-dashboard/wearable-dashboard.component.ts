import {Component, OnDestroy} from '@angular/core';
import { NbThemeService } from '@nebular/theme';
import { takeWhile } from 'rxjs/operators' ;
import { WearableFeedbackService } from '../../../@core/services/wearable-feedback.service';
import { environment } from "../../../../environments/environment";
import { Observable } from 'rxjs';
import { StatusIconRendererComponent } from '../common/goal-table/status-icon-render.component';
import { WearableFeedback } from '../../../@core/data/wearable-feedback';
import { getWearableName } from '../../../@core/utils/poc-utils'
import { getGoalStatusDescription } from '../../../@core/utils/poc-utils'
import { getGoalStatusIcon } from '../../../@core/utils/poc-utils'
import { getSubCodeDescription } from '../../../@core/utils/poc-utils'
import { getPatientNameFromCode } from '../../../@core/utils/poc-utils'
import { getRobotName } from '../../../@core/utils/poc-utils'

interface WearableCardSettings {
  title: string;
  iconClass: string;
  type: string;
  on: boolean;
  bar: number;
  lastUpdate: Date;
  bodyTemperature: number;
  heartRatePerMinute: number;
  oxygenation: number;
  goalStatus: string;
  feedback_obs: Observable<WearableFeedback>
}

@Component({
  selector: 'ngx-wearable-dashboard',
  styleUrls: ['./wearable-dashboard.component.scss'],
  templateUrl: './wearable-dashboard.component.html',
})
// TODO: move status management to another component
export class WearableDashboardComponent implements OnDestroy {

  private alive = true;

  timerStatuses;

  solarValue: number;

  statusCards: string;

  statusCardsSet: WearableCardSettings[] = [];

  statusCardsByThemes: {
    default: WearableCardSettings[]
  } = {
    default: this.statusCardsSet
  };

  goalActions: string[] = ['SOS', 'ABORT'];

  customTableElemMapFunction = (el) => {
    el.statusDesc = getGoalStatusDescription(el.status)
    el.statusIcon = getGoalStatusIcon(el.status)
    el.subCode = getSubCodeDescription(el.subCode)
    el.patientName = getPatientNameFromCode(el.referenceCode)
    if (el.nodeId)
      el.nodeName = getRobotName(el.nodeId)
    if (el.priority == 0)
      el.priorityDesc = "no"
    else 
      el.priorityDesc = el.priority.toString()
    return el;
  };

  columns: any = {
    goalCode: {
      title: 'Codice Goal',
      type: 'string',
    },
    patientName: {
      title: 'Paziente',
      type: 'string',
    },
    statusDesc: {
      title: 'Stato',
      type: 'string',
    },
    subCode: {
      title: 'Tipologia',
      type: 'string',
    },
    priorityDesc: {
      title: 'Priorità',
      type: 'string',
    },
    statusIcon: {
      title: 'Icona stato',
      type: 'custom',
      renderComponent: StatusIconRendererComponent,
      filter: false
    },
    description: {
      title: 'Dettagli',
      type: 'string',
    },
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

  updateWearableStatuses() {
    for (let index = 0; index < this.statusCardsSet.length; index++) {
      const element = this.statusCardsSet[index];

      let subscription_feedback =
        element.feedback_obs
        .subscribe((feedback) => {
          if (feedback) {
            element.type = 'info'
            element.on = true
            element.bar = 100 // at the moment, it's not used
            element.bodyTemperature= feedback.bodyTemperature
            element.heartRatePerMinute= feedback.heartRatePerMinute
            element.oxygenation= feedback.oxygenation
            element.lastUpdate = new Date(feedback.timestamp)
            element.goalStatus = feedback.goalStatus

            // TODO: check these conditions
            if (element.goalStatus == 'RUNNING' ||
                element.bodyTemperature < 35.5 ||
                element.bodyTemperature > 38 ||
                element.oxygenation < 91 ||
                element.heartRatePerMinute <= 70 ||
                element.heartRatePerMinute >= 140) {
                  element.type = 'warning'
              }

            subscription_feedback.unsubscribe();
          }
        });
    }
  }

  constructor(private themeService: NbThemeService,
              private wearableFeedbackService: WearableFeedbackService) {
    this.themeService.getJsTheme()
      .pipe(takeWhile(() => this.alive))
      .subscribe(theme => {
        this.statusCards = this.statusCardsByThemes[theme.name];
    });


    for (let index = 0; index < environment.wearables; index++) {
      const wearableId = index+1;

      let feedback_obs = this.wearableFeedbackService.getLastFeedbackByWearableId(wearableId)

      this.statusCardsSet.push(
        {
          title: getWearableName(wearableId),
          iconClass: 'nb-person',
          type: 'warning',
          feedback_obs: feedback_obs,
          on: false,
          bar: 0.0,
          lastUpdate: undefined,
          bodyTemperature: undefined,
          heartRatePerMinute: undefined,
          oxygenation: undefined,
          goalStatus: ''
        }
      );
    }

    this.timerStatuses = setInterval(() => {
      this.updateWearableStatuses();
    }, 2000);
  }

  ngOnDestroy() {
    this.alive = false;
    if (this.timerStatuses) {
      clearInterval(this.timerStatuses);
    }
  }
}
