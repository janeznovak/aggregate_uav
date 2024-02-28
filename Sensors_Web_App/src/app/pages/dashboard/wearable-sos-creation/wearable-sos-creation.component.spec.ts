import { ComponentFixture, TestBed } from '@angular/core/testing';

import { WearableSOSCreationComponent } from './wearable-sos-creation-creation.component';

describe('GoalCreationComponent', () => {
  let component: WearableSOSCreationComponent;
  let fixture: ComponentFixture<WearableSOSCreationComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ WearableSOSCreationComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(WearableSOSCreationComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
