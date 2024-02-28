import { TestBed } from '@angular/core/testing';

import { WearableSosService } from './wearable-sos.service';

describe('WearableSosService', () => {
  let service: WearableSosService;

  beforeEach(() => {
    TestBed.configureTestingModule({});
    service = TestBed.inject(WearableSosService);
  });

  it('should be created', () => {
    expect(service).toBeTruthy();
  });
});
