package it.unito.sensors.server.dto;

import com.fasterxml.jackson.annotation.JsonInclude;
import lombok.*;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
@JsonInclude(JsonInclude.Include.NON_NULL)
public class CustomWearableSOSDTO implements DTO{

    private String          wearableCode;
    private Integer         priority;
    private String          triggerEvent;
}