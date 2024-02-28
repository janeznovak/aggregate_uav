package it.unito.sensors.server.fs.dto;

import com.opencsv.bean.CsvBindByPosition;
import lombok.*;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@ToString
@Builder
public class CSVFeedbackDTO{

    @CsvBindByPosition(position = 0)
    private String robotName;
    @CsvBindByPosition(position = 1)
    private Float  posX;
    @CsvBindByPosition(position = 2)
    private Float  posY;
    @CsvBindByPosition(position = 3)
    private Float  orientW;
    @CsvBindByPosition(position = 4)
    private Float  batteryChargePercentage;
    @CsvBindByPosition(position = 5)
    private Integer goalStatus;
    @CsvBindByPosition(position = 6)
    private String goalCode;
    @CsvBindByPosition(position = 7)
    private Long unixTimestamp;
}
