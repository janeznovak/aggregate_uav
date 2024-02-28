import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs';
import { environment } from "../../../environments/environment";
import { NodeReport } from '../data/node-report';
import { StatusReport } from '../data/status-report';

@Injectable({
  providedIn: 'root'
})
export class StatsService {

  constructor(private httpClient: HttpClient) { }

  getTotalGoalsByNode(): Observable<NodeReport[]>{
    return this.httpClient.get<NodeReport[]>(environment.base_path_http_server + environment.api_stats_total_goals_by_node);
  }

  getTotalGoalsByStatus(): Observable<StatusReport[]>{
    return this.httpClient.get<StatusReport[]>(environment.base_path_http_server + environment.api_stats_total_goals_by_status);
  }

}
