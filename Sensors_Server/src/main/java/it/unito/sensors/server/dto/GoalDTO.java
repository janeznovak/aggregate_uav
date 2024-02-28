package it.unito.sensors.server.dto;

import com.fasterxml.jackson.annotation.JsonFormat;
import com.fasterxml.jackson.annotation.JsonInclude;
import it.unito.sensors.server.entity.GoalStatusEnum;
import it.unito.sensors.server.entity.GoalSubCodeEnum;
import lombok.*;

import java.time.LocalDateTime;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
@ToString
@JsonInclude(JsonInclude.Include.NON_NULL)
public class GoalDTO implements DTO {

    private String          action;
    private String          goalCode;
    private String          nodeName;
    private Long            nodeId;
    private String          referenceCode;
    private String          source;
    private String          description;
    private Integer         priority;
    private Float           posX;
    private Float           posY;
    private Float           orientW;
    private GoalStatusEnum  status;
    private GoalSubCodeEnum subCode;
    private Boolean         lock;
    @JsonFormat(pattern="yyyy-MM-dd HH:mm:ss")
    private LocalDateTime   timestampUpdate;
    @JsonFormat(pattern="yyyy-MM-dd HH:mm:ss")
    private LocalDateTime   timestampCreation;
}
