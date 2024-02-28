package it.unito.sensors.server.fs.dto;

import com.opencsv.bean.CsvBindByPosition;
import lombok.*;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@ToString
@Builder
public class CSVWearableSosDTO {

    @CsvBindByPosition(position = 0)
    private String wearableCode;
    @CsvBindByPosition(position = 1)
    private Float  posX;
    @CsvBindByPosition(position = 2)
    private Float  posY;
    @CsvBindByPosition(position = 3)
    private Integer  priority;
    @CsvBindByPosition(position = 4)
    private Integer  heartRatePerMinute;
    @CsvBindByPosition(position = 5)
    private Float  bodyTemperature;
    @CsvBindByPosition(position = 6)
    private Integer oxygenation;
    @CsvBindByPosition(position = 7)
    private String  accidentalFallTriggered;
    @CsvBindByPosition(position = 8)
    private String  heartRatePerMinuteTriggered;
    @CsvBindByPosition(position = 9)
    private String  bodyTemperatureTriggered;
    @CsvBindByPosition(position = 10)
    private String oxygenationTriggered;
    @CsvBindByPosition(position = 11)
    private String followTriggered;
    @CsvBindByPosition(position = 12)
    private Long unixTimestamp;
}
