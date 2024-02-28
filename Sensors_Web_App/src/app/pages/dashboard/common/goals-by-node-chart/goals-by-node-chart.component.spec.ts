import { ComponentFixture, TestBed } from '@angular/core/testing';

import { GoalsByNodeChartComponent } from './goals-by-node-chart.component';

describe('GoalsByNodeChartComponent', () => {
  let component: GoalsByNodeChartComponent;
  let fixture: ComponentFixture<GoalsByNodeChartComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ GoalsByNodeChartComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(GoalsByNodeChartComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
