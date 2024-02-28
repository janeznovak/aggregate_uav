import { Component, OnInit, OnDestroy } from '@angular/core';
import { NbThemeService } from '@nebular/theme';
import { StatsService } from '../../../../@core/services/stats.service';
import { getGoalStatusDescription } from '../../../../@core/utils/poc-utils'
import { environment } from "../../../../../environments/environment";
import { setTimeout } from 'timers';
import { serialize } from 'v8';

@Component({
  selector: 'ngx-goals-by-status-chart',
  templateUrl: './goals-by-status-chart.component.html',
  styleUrls: ['./goals-by-status-chart.component.scss']
})
export class GoalsByStatusChartComponent implements OnDestroy {
  data: any;
  options: any;
  themeSubscription: any;
  timer;
  backgroundColors: string[] = [this.getRandomColor(), this.getRandomColor(), this.getRandomColor(), this.getRandomColor(), this.getRandomColor()]

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

  getRandomColor(): string {
    let letters = '0123456789ABCDEF';
    let color = '#';
    for (var i = 0; i < 6; i++) {
      color += letters[Math.floor(Math.random() * 16)];
    }
    return color
}

  refreshChart() {
    this.service
      .getTotalGoalsByStatus()
      .toPromise()
      .then((stats) => {

        let robots = []
        let values = []

        for (const obj of stats) {
          robots.push(getGoalStatusDescription(obj.status))
          values.push(obj.value)
        }

        this.data = {
          labels: robots,
          datasets: [{
            data: values,
            backgroundColor: this.backgroundColors,
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
