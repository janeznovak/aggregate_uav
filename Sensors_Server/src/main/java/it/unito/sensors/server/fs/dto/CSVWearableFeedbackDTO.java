package it.unito.sensors.server.fs.dto;

import com.opencsv.bean.CsvBindByPosition;
import lombok.*;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@ToString
@Builder
public class CSVWearableFeedbackDTO {

    @CsvBindByPosition(position = 0)
    private String wearableCode;
    @CsvBindByPosition(position = 1)
    private Float  posX;
    @CsvBindByPosition(position = 2)
    private Float  posY;
    @CsvBindByPosition(position = 3)
    private Integer  heartRatePerMinute;
    @CsvBindByPosition(position = 4)
    private Float  bodyTemperature;
    @CsvBindByPosition(position = 5)
    private Integer oxygenation;
    @CsvBindByPosition(position = 6)
    private Integer goalStatus;
    @CsvBindByPosition(position = 7)
    private String goalCode;
    @CsvBindByPosition(position = 8)
    private Long unixTimestamp;
}
