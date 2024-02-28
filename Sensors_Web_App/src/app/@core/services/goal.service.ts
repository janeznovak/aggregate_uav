import { Injectable } from '@angular/core';
import { Goal } from '../../@core/data/goal';
import { Observable } from 'rxjs';
import {environment} from "../../../environments/environment";
import { HttpClient,HttpParams } from '@angular/common/http';

@Injectable({
  providedIn: 'root'
})
export class GoalService {

  constructor(private httpClient: HttpClient) { }

  getGoals(actions:string[]): Observable<Goal[]>{
    return this.httpClient.get<Goal[]>(
      environment.base_path_http_server + environment.api_goals, 
      { 
        params: actions.reduce((accumulator, name) => accumulator.append('actions', name), new HttpParams())
      }
    );
  }

  createActionGoal(itemName: string): Observable<Goal>{
    return this.httpClient.post<Goal>(environment.base_path_http_server + environment.api_goals, {
      action: "GOAL",
      goalCode: "GOAL-" + new Date().getTime(),
      referenceCode: itemName
    });
  }

  createActionAbort(goalCode: string): Observable<Goal>{
    return this.httpClient.post<Goal>(environment.base_path_http_server + environment.api_goals, {
      action: "ABORT",
      goalCode: goalCode
    });
  }
}
