package it.unito.sensors.server.fs.dto;

import com.opencsv.bean.CsvBindByPosition;
import lombok.*;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@ToString
@Builder
public class CSVActionDTO {

    @CsvBindByPosition(position = 0)
    private String  action;
    @CsvBindByPosition(position = 1)
    private String  goal;
    @CsvBindByPosition(position = 2)
    private String  robot;
    /* ... goal data not used... */
    @CsvBindByPosition(position = 6)
    private Long unixTimestamp;
}
