package it.unito.sensors.server.dto;

import com.fasterxml.jackson.annotation.JsonFormat;
import com.fasterxml.jackson.annotation.JsonInclude;
import it.unito.sensors.server.entity.MeasurementData;
import it.unito.sensors.server.entity.NeighbourEntity;
import it.unito.sensors.server.entity.NodeEntity;
import lombok.*;

import java.time.LocalDateTime;
import java.util.List;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
@ToString
@JsonInclude(JsonInclude.Include.NON_NULL)
public class MeasurementDTO implements DTO {

    private Long                    id;
    private NodeEntity              nodeEntity;
    private Long                    unixTimestamp;
    private String                  nodeName;
    @JsonFormat(pattern="yyyy-MM-dd HH:mm:ss")
    private LocalDateTime           timestamp;
    private List<NeighbourEntity>   neighbours;
    private MeasurementData         measurementData;

}
