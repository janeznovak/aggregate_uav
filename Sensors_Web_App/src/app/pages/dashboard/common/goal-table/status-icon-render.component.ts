import { Component, Input, OnInit } from '@angular/core';
import { ViewCell } from 'ng2-smart-table';

// TODO: refactor using pipe!
@Component({
  template: `
  <span style="color:{{ color }}"><i class="{{ icon }}" style="font-size: 30px;"></i></span>
  `,
})
export class StatusIconRendererComponent implements ViewCell, OnInit {
  @Input() value: any;    // This hold the cell value
  @Input() rowData: any;  // This holds the entire row object
  icon: string
  color: string

  ngOnInit() {
    this.icon = this.value.split(',')[0]  
    this.color = this.value.split(',')[1]  
  }

}