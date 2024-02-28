import { Component, OnInit, OnDestroy } from '@angular/core';
import { NbThemeService } from '@nebular/theme';
import { StatsService } from '../../../../@core/services/stats.service';
import { getRobotName } from '../../../../@core/utils/poc-utils'
import { environment } from "../../../../../environments/environment";
import { setTimeout } from 'timers';
import { serialize } from 'v8';

@Component({
  selector: 'ngx-goals-by-node-chart',
  templateUrl: './goals-by-node-chart.component.html',
  styleUrls: ['./goals-by-node-chart.component.scss']
})
export class GoalsByNodeChartComponent implements OnDestroy {
  data: any;
  options: any;
  themeSubscription: any;
  timer;

  constructor(private theme: NbThemeService, 
              private service: StatsService) {
    this.themeSubscription = this.theme.getJsTheme().subscribe(config => {

      const chartjs: any = config.variables.chartjs;

      this.options = {
        maintainAspectRatio: false,
        responsive: true,
        scales: {
          xAxes: [
            {
              display: false,
            },
          ],
          yAxes: [
            {
              display: false,
            },
          ],
        },
        legend: {
          labels: {
            fontColor: chartjs.textColor,
          },
        },
      };
    });

    this.refreshChart()
    this.timer = setInterval(() => {
      this.refreshChart(); 
    }, 10000);
  }

  refreshChart() {
    this.service
      .getTotalGoalsByNode()
      .toPromise()
      .then((stats) => {

        let robots = []
        let values = []

        for (const obj of stats) {
          robots.push(getRobotName(obj.nodeId))
          values.push(obj.value)
        }

        this.data = {
          labels: robots,
          datasets: [{
            data: values,
            backgroundColor: environment.robots_color,
          }],
        };
      }) 
      .finally(() => { 
        
      })
      .catch((err) => { 
        console.error(err)
      })
  }

  ngOnDestroy(): void {
    this.themeSubscription.unsubscribe();
    if (this.timer) {
      clearInterval(this.timer);
    }
  }
}
