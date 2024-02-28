export interface WearableFeedback {
  wearableCode: string;
  referenceCode: string;
  posX: number;
  posY: number;
  heartRatePerMinute: number;
  bodyTemperature: number;
  oxygenation: number;
  goalStatus: string;
  timestamp: Date;
}
