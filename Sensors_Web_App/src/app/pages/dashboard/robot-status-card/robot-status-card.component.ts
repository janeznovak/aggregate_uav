import { Component, Input, OnChanges } from '@angular/core';
import { DatePipe } from '@angular/common';
import { NbIconLibraries } from '@nebular/theme';

@Component({
  selector: 'ngx-robot-status-card',
  styleUrls: ['./robot-status-card.component.scss'],
  template: `
    <nb-card [ngClass]="{'off': !on}">
      <div class="icon-container">
        <div class="icon status-{{ type }}">
          <ng-content></ng-content>
        </div>
      </div>

      <div class="details">
        <div class="title h5">{{ title }}</div>
        <div class="status paragraph-2">{{ on ? 'ON' : 'OFF' }}</div>
        <div class="bar">
          <nb-progress-bar [value]="bar" status="{{status}}" [displayValue]="true">
          </nb-progress-bar>
        </div>
        <div class="update">
          Aggiornamento: {{ lastUpdate }}
        </div>
      </div>

      <div class="other-details">
        <div class="measurements">
          <div *ngIf="temperature">{{ temperature }}°C <nb-icon class="icons" icon="temperature-quarter" pack="fa"></nb-icon></div>
          <div *ngIf="humidity">{{ humidity }}% <nb-icon class="icons" icon="droplet" pack="fa"></nb-icon></div>
          <div *ngIf="pressure">{{ pressure }}mb <nb-icon class="icons" icon="gauge" pack="fa"></nb-icon></div>
        </div>
      </div>
    </nb-card>
  `,
})
export class RobotStatusCardComponent implements OnChanges{

  @Input() title: string;
  @Input() type: string;
  @Input() on: boolean = true;
  @Input() bar: number = 0.0;
  @Input() lastUpdate: string;
  @Input() temperature: number;
  @Input() humidity: number;
  @Input() pressure: number;

  status: string = 'primary'

  constructor(private datePipe: DatePipe,
              iconsLibrary: NbIconLibraries) {
    iconsLibrary.registerFontPack('fa', { packClass: 'fa', iconClassPrefix: 'fa' });
  }

  ngOnChanges(): void {
    if (this.bar > 50.0) {
      this.status = 'primary'
    } else if (this.bar <= 50.0 && this.bar >= 30.0) {
      this.status = 'warning'
    } else {
      this.status = 'danger'
    }
    if (this.bar < 0) {
      this.bar = 0
    }

    if (this.lastUpdate == undefined) {
      this.lastUpdate = "non presente"
    } else {
      this.lastUpdate = this.datePipe.transform(this.lastUpdate, 'HH:mm:ss.SSS');;
    }
  }
}
