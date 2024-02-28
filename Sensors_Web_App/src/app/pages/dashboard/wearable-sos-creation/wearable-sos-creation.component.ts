import { Component, OnInit, OnDestroy } from '@angular/core';
import { UntypedFormBuilder, UntypedFormGroup, Validators } from '@angular/forms';
import { NbStepperComponent, NbStepComponent } from '@nebular/theme';
import { ViewChild } from '@angular/core'
import { NbStepChangeEvent } from '@nebular/theme';
import { WearableSosService } from '../../../@core/services/wearable-sos.service';
import { Goal } from '../../../@core/data/goal';
import { environment } from "../../../../environments/environment";
import { getWearableName } from '../../../@core/utils/poc-utils'

interface PatientOption {
  value: string;
  label: string;
}

@Component({
  selector: 'ngx-wearable-sos-creation',
  templateUrl: './wearable-sos-creation.component.html',
  styleUrls: ['./wearable-sos-creation.component.scss']
})
export class WearableSOSCreationComponent implements OnInit {
  @ViewChild('stepper') stepper: NbStepperComponent;
  @ViewChild('errorstep') errorstep: NbStepComponent;

  patientOptions: PatientOption[] = [];

  wearableCode: string = ""
  triggerEvent: string = ""
  priority: number
  firstForm: UntypedFormGroup;
  secondForm: UntypedFormGroup;
  thirdForm: UntypedFormGroup;
  changeEvent: NbStepChangeEvent;

  constructor(private fb: UntypedFormBuilder, private service: WearableSosService) {
  }

  private buildPatientOptions(): void {
    for (let i = 1; i <= environment.wearables; i++) {
      let optionValue: string = `wb_${i}`;
      let optionLabel: string = getWearableName(i);
      this.patientOptions.push({ value: optionValue, label: optionLabel });
    }
  }

  handleStepChange(e: NbStepChangeEvent): void {
    let that = this
    this.changeEvent = e;

    // step code
    if (this.changeEvent.index == 2) {

      let subscription =
        this.service
        .createCustomWearableSOS(this.wearableCode, this.triggerEvent, this.priority)
        .subscribe
        ({
          next(f: boolean) {
            console.log("wearable sos creation ok");
          },
          complete() {
            setTimeout(() => {
              that.stepper.next()
              subscription.unsubscribe()
            }, 500);
          },
          error(e) {
            // show error
            console.log(e)
            // it doesn't work
            // that.stepper.changeStep(that.errorstep)
            that.stepper.next()
            that.stepper.next()
            subscription.unsubscribe()
          }
        });
    }
  }

  ngOnInit() {
    this.buildPatientOptions()
    this.firstForm = this.fb.group({
      firstCtrl: ['', Validators.required],
    });

    this.thirdForm = this.fb.group({
      thirdCtrl: ['', Validators.required],
    });
  }

  onFirstSubmit() {
    this.firstForm.markAsDirty();
  }

}
