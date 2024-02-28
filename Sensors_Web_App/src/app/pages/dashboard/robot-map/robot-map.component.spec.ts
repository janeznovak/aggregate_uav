import { ComponentFixture, TestBed } from '@angular/core/testing';

import { RobotMapComponent } from './robot-map.component';

describe('RobotMapComponent', () => {
  let component: RobotMapComponent;
  let fixture: ComponentFixture<RobotMapComponent>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      declarations: [ RobotMapComponent ]
    })
    .compileComponents();

    fixture = TestBed.createComponent(RobotMapComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
