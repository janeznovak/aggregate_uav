package it.unito.sensors.server.dto.mapper;

import it.unito.sensors.server.dto.MeasurementDTO;
import it.unito.sensors.server.entity.MeasurementEntity;
import org.springframework.stereotype.Component;

@Component
public class MeasurementDTOMapper implements DTOMapper<MeasurementEntity, MeasurementDTO>{

    public MeasurementDTO map(MeasurementEntity measurementEntity) {
        return MeasurementDTO.builder()
                .nodeName(measurementEntity.getNodeEntity().getName())
                .timestamp(measurementEntity.getTimestamp())
                .unixTimestamp(measurementEntity.getUnixTimestamp())
                .neighbours(measurementEntity.getNeighbours())
                .measurementData(measurementEntity.getMeasurementData())
                .build();
    }
}
