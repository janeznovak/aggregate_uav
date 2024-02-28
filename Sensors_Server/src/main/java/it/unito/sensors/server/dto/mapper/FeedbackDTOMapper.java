package it.unito.sensors.server.dto.mapper;

import it.unito.sensors.server.dto.FeedbackDTO;
import it.unito.sensors.server.entity.FeedbackEntity;
import it.unito.sensors.server.entity.NodeEntity;
import org.springframework.stereotype.Component;

import java.util.Optional;

@Component
public class FeedbackDTOMapper implements DTOMapper<FeedbackEntity, FeedbackDTO> {

    @Override
    public FeedbackDTO map(FeedbackEntity feedbackEntity) {
        return FeedbackDTO.builder()
                .posX(feedbackEntity.getPosX())
                .posY(feedbackEntity.getPosY())
                .orientW(feedbackEntity.getOrientW())
                .timestamp(feedbackEntity.getTimestamp())
                .batteryPercentCharge(feedbackEntity.getBatteryPercentCharge())
                .goalStatus(feedbackEntity.getGoalStatus())
                .nodeName(Optional.ofNullable(feedbackEntity.getNodeEntity()).map(NodeEntity::getName).orElse(null))
                .build();
    }
}
