import { ComponentFixture, TestBed } from '@angular/core/testing';

import { GoalCreationComponent } from './goal-creation.component';

describe('GoalCreationComponent', () => {
  let component: GoalCreationComponent;
  let fixture: ComponentFixture<GoalCreationComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ GoalCreationComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(GoalCreationComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
