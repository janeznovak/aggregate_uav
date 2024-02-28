package it.unito.sensors.server.dto.mapper;

import it.unito.sensors.server.dto.GoalDTO;
import it.unito.sensors.server.entity.GoalEntity;
import it.unito.sensors.server.entity.NodeEntity;
import org.springframework.stereotype.Component;

import java.util.Optional;

@Component
public class GoalDTOMapper implements DTOMapper<GoalEntity, GoalDTO> {

    public GoalDTO map(GoalEntity goalEntity) {
        return GoalDTO.builder()
                .goalCode(goalEntity.getCode())
                .posX(goalEntity.getPosX())
                .posY(goalEntity.getPosY())
                .action(goalEntity.getAction())
                .orientW(goalEntity.getOrientW())
                .referenceCode(goalEntity.getReferenceCode())
                .source(goalEntity.getSource())
                .description(goalEntity.getDescription())
                .priority(goalEntity.getPriority())
                .status(goalEntity.getStatus())
                .subCode(goalEntity.getSubCode())
                .description(goalEntity.getDescription())
                .lock(goalEntity.getLock())
                .timestampCreation(goalEntity.getTimestampCreation())
                .timestampUpdate(goalEntity.getTimestampUpdate())
                .nodeId(Optional.ofNullable(goalEntity.getNodeEntity()).map(NodeEntity::getId).orElse(null))
                .nodeName(Optional.ofNullable(goalEntity.getNodeEntity()).map(NodeEntity::getName).orElse(null))
                .build();
    }
}
