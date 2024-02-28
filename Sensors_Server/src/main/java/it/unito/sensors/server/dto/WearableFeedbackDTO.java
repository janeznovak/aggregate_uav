package it.unito.sensors.server.dto;

import com.fasterxml.jackson.annotation.JsonFormat;
import com.fasterxml.jackson.annotation.JsonInclude;
import it.unito.sensors.server.entity.GoalStatusEnum;
import lombok.*;

import java.time.LocalDateTime;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
@JsonInclude(JsonInclude.Include.NON_NULL)
public class WearableFeedbackDTO implements DTO{

    private String          wearableCode;
    private String          referenceCode;
    private Float           posX;
    private Float           posY;
    private Integer         heartRatePerMinute;
    private Float           bodyTemperature;
    private Integer         oxygenation;
    private GoalStatusEnum  goalStatus;
    @JsonFormat(pattern="yyyy-MM-dd HH:mm:ss")
    private LocalDateTime   timestamp;
}