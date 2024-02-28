import { ComponentFixture, TestBed } from '@angular/core/testing';

import { GoalsByStatusChartComponent } from './goals-by-status-chart.component';

describe('GoalsByStatusChartComponent', () => {
  let component: GoalsByStatusChartComponent;
  let fixture: ComponentFixture<GoalsByStatusChartComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ GoalsByStatusChartComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(GoalsByStatusChartComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
