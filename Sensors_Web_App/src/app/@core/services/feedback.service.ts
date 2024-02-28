import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Feedback } from '../../@core/data/feedback';
import { Observable } from 'rxjs';
import {environment} from "../../../environments/environment";

@Injectable({
  providedIn: 'root'
})
export class FeedbackService {

  constructor(private httpClient: HttpClient) { }

  getLastFeedbackByNodeId(nodeId: number): Observable<Feedback>{
    return this.httpClient.get<Feedback>(environment.base_path_http_server + environment.api_feedbacks_last_by_node.replace(environment.api_node_id_placehoder, nodeId.toString()));
  }
}
