import { Injectable } from '@angular/core';
import { WearableSOS } from '../../@core/data/wearable-sos';
import { Observable } from 'rxjs';
import {environment} from "../../../environments/environment";
import { HttpClient,HttpParams } from '@angular/common/http';

@Injectable({
  providedIn: 'root'
})
export class WearableSosService {

  constructor(private httpClient: HttpClient) { }

  createCustomWearableSOS(wearableCode: string, triggerEvent: string, priority: number): Observable<boolean>{
    return this.httpClient.post<boolean>(environment.base_path_http_server + environment.api_wearable_sos, {
      wearableCode: wearableCode,
      triggerEvent: triggerEvent,
      priority: priority
    });
  }

}
