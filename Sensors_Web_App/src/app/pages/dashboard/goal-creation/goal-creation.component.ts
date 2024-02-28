import { Component, OnInit, OnDestroy } from '@angular/core';
import { UntypedFormBuilder, UntypedFormGroup, Validators } from '@angular/forms';
import { NbStepperComponent, NbStepComponent } from '@nebular/theme';
import { ViewChild } from '@angular/core'
import { NbStepChangeEvent } from '@nebular/theme';
import { GoalService } from '../../../@core/services/goal.service';
import { Goal } from '../../../@core/data/goal';

@Component({
  selector: 'ngx-goal-creation',
  templateUrl: './goal-creation.component.html',
  styleUrls: ['./goal-creation.component.scss']
})
export class GoalCreationComponent implements OnInit {
  @ViewChild('stepper') stepper: NbStepperComponent;
  @ViewChild('errorstep') errorstep: NbStepComponent;
  itemName: string
  goalCode: string
  firstForm: UntypedFormGroup;
  secondForm: UntypedFormGroup;
  thirdForm: UntypedFormGroup;
  changeEvent: NbStepChangeEvent;

  constructor(private fb: UntypedFormBuilder, private service: GoalService) {
  }

  handleStepChange(e: NbStepChangeEvent): void {
    let that = this
    this.changeEvent = e;
    
    // step code
    if (this.changeEvent.index == 2) {

      let subscription = 
        this.service
        .createActionGoal(this.itemName)
        .subscribe
        ({
          next(f: Goal) { 
            that.goalCode = f.goalCode
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