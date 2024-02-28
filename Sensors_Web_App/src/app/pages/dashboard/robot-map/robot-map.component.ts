import { Component, OnDestroy, OnInit } from '@angular/core';
import { DrawProps, ImageProps, TextProps, PropagateProps, RectProps, LineProps, StepsProps, Types } from 'ngx-canvas';
import { FeedbackService } from '../../../@core/services/feedback.service';
import { getRobotName } from '../../../@core/utils/poc-utils'
import { Feedback } from '../../../@core/data/feedback';
import { environment} from "../../../../environments/environment";


@Component({
  selector: 'ngx-robot-map',
  templateUrl: './robot-map.component.html',
  styleUrls: ['./robot-map.component.scss']
})
export class RobotMapComponent implements OnInit, OnDestroy {
  subscription: any
  autoReload: boolean = true;
  options: DrawProps;
  lines: LineProps[] = []
  txts: TextProps[] = []
  rects: RectProps[] = []
  colors: string[] = environment.robots_color
  last_positions: [number, number][] = []
  timer
  canvas_x_length: number
  canvas_y_length: number
  canvas_axis_conversion_factor: number = 75

  // items coordinates
  items: [number, number][] = []

  constructor(private service: FeedbackService) {
    for (let index = 0; index < environment.robots; index++) {
      this.last_positions.push([NaN, NaN])
    }
    this.canvas_x_length = environment.axis_x_length * this.canvas_axis_conversion_factor
    this.canvas_y_length = environment.axis_y_length * this.canvas_axis_conversion_factor
  }

  private convertCoordinateToCanvas(pos: [number, number]) : [number, number] {
    return [pos[0] * this.canvas_axis_conversion_factor, this.canvas_y_length -(pos[1] * this.canvas_axis_conversion_factor)]
  }

  initializeCanvas() {
    // draw image
    const imgs: ImageProps[] = [];
    // draw text
    // for (let index = 0; index < environment.items; index++) {
    //   const element = index+1;
    //   const item = this.items[index]

    //   this.txts.push(
    //     {
    //       type: 'text',
    //       top: item[1],
    //       left: item[0],
    //       content: '('+element+')',
    //       fontSize: 12,
    //       fontWeight: '1',
    //       color: '#000000',
    //     },
    //   );
    // }
    // draw rect
    for (let index = 0; index < environment.items; index++) {
      const element = index+1;
      const item = this.items[index]

      this.rects.push(
        {
          type: Types.rect,
          width: 20,
          height: 20,
          x: item[0],
          y: item[1],
          backgroundColor: '',
          borderColor: '#000000',
          borderWidth: 1,
          text: {
                  type: 'text',
                  top: item[1]+5,
                  left: item[0]+5,
                  content: ''+element,
                  width: 0,
                  fontSize: 12,
                  fontWeight: '1',
                  color: '#000000',
                }
        }
      );
    }

    // draw steps
    const steps: StepsProps[] = [];
    // draw options
    this.refreshOptions(this.lines, this.txts)
  }

  ngOnInit(): void {
    this.items.push(this.convertCoordinateToCanvas([0.5, 0.5]))
    this.items.push(this.convertCoordinateToCanvas([0.5, 1.0]))
    this.items.push(this.convertCoordinateToCanvas([0.5, 2.2]))
    this.items.push(this.convertCoordinateToCanvas([0.5, 3.0]))
    this.items.push(this.convertCoordinateToCanvas([0.5, 4.2]))
    this.items.push(this.convertCoordinateToCanvas([0.5, 5.0]))
    this.items.push(this.convertCoordinateToCanvas([0.5, 6.2]))
    this.items.push(this.convertCoordinateToCanvas([0.5, 6.7]))

    this.initializeCanvas();
    this.timer = setInterval(() => {
      this.refreshRobotMap(); 
    }, 500);
  }

  ngOnDestroy() {
    if (this.timer) {
      clearInterval(this.timer);
    }
  }

  refreshOptions(new_lines: LineProps[], new_texts: TextProps[]) {
    this.options = {
      debug: true, // Used to debug the presentation canvas
      width: this.canvas_x_length,
      height: this.canvas_y_length,
      backgroundColor: '#FFFFFF',
      views: [
        ...new_texts,
        ...this.rects,
        ...new_lines
      ],
    };
  }

  refreshRobotMap() {
    if (!this.autoReload)
      return;

    let that = this

    // TODO: refactor call "refreshOptions" once, after ALL ajax call
    that.txts.splice(0)
    for (let index = 0; index < environment.robots; index++) {  
      let nodeId = index+1   
      this.service
        .getLastFeedbackByNodeId(nodeId)
        .toPromise()
        .then((feedback) => { 
          if (feedback) {
            let new_pos = that.convertCoordinateToCanvas([feedback.posX, feedback.posY])
            let new_pos_x = new_pos[0]
            let new_pos_y = new_pos[1]
            if (that.last_positions[nodeId-1]) {
              that.lines.push(
                {
                  type: Types.line,
                  color: that.colors[nodeId-1],
                  startX: that.last_positions[nodeId-1][0],
                  startY: that.last_positions[nodeId-1][1],
                  endX: new_pos_x,
                  endY: new_pos_y,
                  width: 7,
                  lineCap: 'round'
                }
              );
            } else {
              that.lines.push(
                {
                  type: Types.line,
                  color: that.colors[nodeId-1],
                  startX: new_pos_x+1,
                  startY: new_pos_x+1,
                  endX: new_pos_x,
                  endY: new_pos_y,
                  width: 7,
                  lineCap: 'round'
                }
              );
            }
            that.txts.push(
              {
                type: 'text',
                top: new_pos_y + 5,
                left: new_pos_x - 15,
                content: getRobotName(nodeId),
                width: 50,
                fontSize: 15,
                fontWeight: '1',
                color: '#000000',
              }
            );
            // refresh last position
            that.last_positions[nodeId-1] = [new_pos_x, new_pos_y]
          }
        }) 
        .finally(() => { 
          // TODO: fix canvas blinking
          // that.refreshOptions(that.lines, that.txts)
        })
        .catch((err) => { 
          console.error(err)
        })
    }
    setTimeout(() => {
      this.refreshOptions(this.lines, this.txts)
    }, 50)
  }

  drawComplete(propagate: PropagateProps): void {
    const { dataUrl, canvas, ctx, extra } = propagate;
  }
}