import { Component, Input, OnChanges } from '@angular/core';
import { DatePipe } from '@angular/common';
import { NbIconLibraries } from '@nebular/theme';

@Component({
  selector: 'ngx-wearable-status-card',
  styleUrls: ['./wearable-status-card.component.scss'],
  template: `
    <nb-card [ngClass]="{'off': !on}">
      <div class="icon-container">
        <div class="icon status-{{ type }}">
          <ng-content></ng-content>
        </div>
      </div>

      <div class="details">
        <div class="title h5">{{ title }}</div>
        <div class="status paragraph-2">{{ on ? '' : 'off' }}</div>
        <div class="update">
          Aggiornamento: {{ lastUpdate }}
        </div>
      </div>

      <div class="other-details">
        <div class="measurements">
          <div *ngIf="bodyTemperature">{{ bodyTemperature }}°C <nb-icon class="icons" icon="temperature-quarter" pack="fa"></nb-icon></div>
          <div *ngIf="heartRatePerMinute">{{ heartRatePerMinute }} per minuto <nb-icon class="icons" icon="heart-pulse" pack="fa"></nb-icon></div>
          <div *ngIf="oxygenation">{{ oxygenation }} <nb-icon class="icons" icon="mask-ventilator" pack="fa"></nb-icon></div>
        </div>
      </div>
    </nb-card>
  `,
})
export class WearableStatusCardComponent implements OnChanges{

  @Input() title: string;
  @Input() type: string;
  @Input() on: boolean = true;
  @Input() bar: number = 0.0;
  @Input() goalStatus: string;
  @Input() lastUpdate: string;
  @Input() bodyTemperature: number;
  @Input() heartRatePerMinute: number;
  @Input() oxygenation: number;

  status: string = 'primary'

  constructor(private datePipe: DatePipe,
              iconsLibrary: NbIconLibraries) {
    iconsLibrary.registerFontPack('fa', { packClass: 'fa', iconClassPrefix: 'fa' });
  }

  ngOnChanges(): void {
    // if (this.bar > 50.0) {
    //   this.status = 'primary'
    // } else if (this.bar <= 50.0 && this.bar >= 30.0) {
    //   this.status = 'warning'
    // } else {
    //   this.status = 'danger'
    // }
    // if (this.bar < 0) {
    //   this.bar = 0
    // }

    if (this.lastUpdate == undefined) {
      this.lastUpdate = "non presente"
    } else {
      this.lastUpdate = this.datePipe.transform(this.lastUpdate, 'HH:mm:ss.SSS');;
    }
  }
}
