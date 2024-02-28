import { Input, Component, SimpleChanges } from '@angular/core';
import { LocalDataSource } from 'ng2-smart-table';

import { GoalService } from '../../../../@core/services/goal.service';
import { Goal } from '../../../../@core/data/goal';
import { Observable } from 'rxjs';
import { StatusIconRendererComponent } from './status-icon-render.component'
import { getGoalStatusDescription } from '../../../../@core/utils/poc-utils'
import { setTimeout } from 'timers';
import { serialize } from 'v8';

@Component({
  selector: 'ngx-goal-table',
  templateUrl: './goal-table.component.html',
  styleUrls: ['./goal-table.component.scss'],
})
export class GoalTableComponent {

  @Input() actions: string[] = [];
  @Input() columns: any;
  @Input() customTableElemMapFunction: (data: any) => any;

  timer;
  settings = {
    actions: {
      add: false,
      edit: false,
      delete: false,
      custom: [
        { name: 'abort', title: '<i class="nb-trash abort-goal-icon"></i>'},
      ]
    },
    rowClassFunction: (row) => {
      if (row.data.statusDesc !== getGoalStatusDescription('RUNNING')) {
        return 'hide-abort-icon';
      }
      return '';
    },
    columns: {},
  };

  source: LocalDataSource = new LocalDataSource();
  goals$: Observable<Goal[]>;

  onCustomAction(event) {
    if (event.action == 'abort') {
      if (event.data.status == 'RUNNING') {
        alert("Sei sicuro di voler cancellare il goal " + event.data.goalCode + "?")
        this.service
          .createActionAbort(event.data.goalCode)
          .toPromise()
          .then((goal) => {
          })
          .finally(() => {
          })
          .catch((err) => {
            console.error(err)
          })

      } else {
        console.warn("can not abort a not running goal")
      }
    }
  }

  refreshGoals() {
    this.goals$
    .subscribe((data) => {
      let new_data = data.map(this.customTableElemMapFunction);
      this.source.load(new_data);
    });
  }

  constructor(private service: GoalService) {
  }

  ngOnChanges(changes: SimpleChanges) {
    if ('actions' in changes) {
      this.goals$ = this.service.getGoals(this.actions);
      this.timer = setInterval(() => {
        this.refreshGoals();
      }, 500);
    }
    if ('columns' in changes) {
      this.settings.columns = this.columns;
    }
  }

  ngOnDestroy(): void {
    if (this.timer) {
      clearInterval(this.timer);
    }
  }
}
