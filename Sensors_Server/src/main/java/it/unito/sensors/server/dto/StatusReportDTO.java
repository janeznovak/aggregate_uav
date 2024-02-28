package it.unito.sensors.server.dto;

import com.fasterxml.jackson.annotation.JsonInclude;
import it.unito.sensors.server.entity.EnumCode;
import lombok.*;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
@ToString
@JsonInclude(JsonInclude.Include.NON_NULL)
public class StatusReportDTO {
    private String status;
    private Long value;

    public StatusReportDTO(EnumCode en, Long value) {
        this.status = en.getCode();
        this.value = value;
    }
}
