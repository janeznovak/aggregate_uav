import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Measurement } from '../../@core/data/measurement';
import { Observable } from 'rxjs';
import {environment} from "../../../environments/environment";

@Injectable({
  providedIn: 'root'
})
export class MeasurementService {

  constructor(private httpClient: HttpClient) { }

  getLastMeasurementByNodeId(nodeId: number): Observable<Measurement>{
    return this.httpClient.get<Measurement>(environment.base_path_http_server + environment.api_measurements_last_by_node.replace(environment.api_node_id_placehoder, nodeId.toString()));
  }
}
