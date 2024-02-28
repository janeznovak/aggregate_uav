import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { WearableFeedback } from '../../@core/data/wearable-feedback';
import { Observable } from 'rxjs';
import {environment} from "../../../environments/environment";

@Injectable({
  providedIn: 'root'
})
export class WearableFeedbackService {

  constructor(private httpClient: HttpClient) { }

  getLastFeedbackByWearableId(wearableId: number): Observable<WearableFeedback>{
    return this.httpClient.get<WearableFeedback>(environment.base_path_http_server + environment.api_wearable_feedbacks_last_by_wearable.replace(environment.api_wearable_id_placehoder, wearableId.toString()));
  }
}
