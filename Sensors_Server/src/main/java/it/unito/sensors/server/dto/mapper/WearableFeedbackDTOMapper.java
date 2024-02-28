package it.unito.sensors.server.dto.mapper;

import it.unito.sensors.server.dto.WearableFeedbackDTO;
import it.unito.sensors.server.entity.WearableEntity;
import it.unito.sensors.server.entity.WearableFeedbackEntity;
import org.springframework.stereotype.Component;

import java.util.Optional;

@Component
public class WearableFeedbackDTOMapper implements DTOMapper<WearableFeedbackEntity, WearableFeedbackDTO> {

    @Override
    public WearableFeedbackDTO map(WearableFeedbackEntity wearableFeedbackEntity) {
        return WearableFeedbackDTO.builder()
                .posX(wearableFeedbackEntity.getPosX())
                .posY(wearableFeedbackEntity.getPosY())
                .heartRatePerMinute(wearableFeedbackEntity.getHeartRatePerMinute())
                .bodyTemperature(wearableFeedbackEntity.getBodyTemperature())
                .oxygenation(wearableFeedbackEntity.getOxygenation())
                .goalStatus(wearableFeedbackEntity.getGoalStatus())
                .timestamp(wearableFeedbackEntity.getTimestamp())
                .wearableCode(Optional.ofNullable(wearableFeedbackEntity.getWearableEntity()).map(WearableEntity::getCode).orElse(null))
                .build();
    }
}
