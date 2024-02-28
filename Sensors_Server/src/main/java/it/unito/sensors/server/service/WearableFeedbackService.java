package it.unito.sensors.server.service;

import it.unito.sensors.server.configuration.AppConfiguration;
import it.unito.sensors.server.entity.GoalStatusEnum;
import it.unito.sensors.server.entity.WearableEntity;
import it.unito.sensors.server.entity.WearableFeedbackEntity;
import it.unito.sensors.server.fs.dto.CSVWearableFeedbackDTO;
import it.unito.sensors.server.repository.WearableFeedbackRepository;
import it.unito.sensors.server.repository.WearableRepository;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.util.TimeZone;

/**
 * \brief FeedbackWearableService exposes to other services some extended features about actions.
 */
@Service
@Slf4j
public class WearableFeedbackService {

    @Autowired
    AppConfiguration appConfiguration;

    @Autowired
    WearableFeedbackRepository wearableFeedbackRepository;

    @Autowired
    WearableRepository wearableRepository;

    @Autowired
    GoalService goalService;

    public WearableFeedbackService(AppConfiguration appConfiguration) {
        this.appConfiguration = appConfiguration;
    }

    /**
     * Save a new wearable feedback
     * @param csvWearableFeedbackDTO
     */
    public void saveNewFeedback(CSVWearableFeedbackDTO csvWearableFeedbackDTO) {
        WearableEntity wearableEntity = wearableRepository.findByCode(csvWearableFeedbackDTO.getWearableCode());
        if (wearableEntity == null) {
            log.warn("request to join with an unknown wearable: {}", csvWearableFeedbackDTO.getWearableCode());
            return;
        }

        WearableFeedbackEntity lastWearableFeedback = wearableFeedbackRepository.findTop1ByWearableEntityOrderByUnixTimestampDesc(wearableEntity);
        if (lastWearableFeedback != null) {
//            log.info("request to persist feedback: last feedback {} vs new feedback {}", lastFeedback.getUnixTimestamp(), csvWearableFeedbackDTO.getUnixTimestamp());
            if (lastWearableFeedback.getUnixTimestamp() > csvWearableFeedbackDTO.getUnixTimestamp()) {
                log.warn("request to persist an old wearable feedback: last feedback {} > new feedback {}", lastWearableFeedback.getUnixTimestamp(), csvWearableFeedbackDTO.getUnixTimestamp());
            }
        }

        LocalDateTime triggerTime = LocalDateTime.now();
        if (csvWearableFeedbackDTO.getUnixTimestamp() != null) {
            triggerTime = LocalDateTime.ofInstant(
                    Instant.ofEpochMilli(csvWearableFeedbackDTO.getUnixTimestamp()),
                    TimeZone.getDefault().toZoneId()
            );
        }

        GoalStatusEnum goalStatus = GoalStatusEnum.NO_GOAL;
        if (csvWearableFeedbackDTO.getGoalStatus() >= 0) {
            goalStatus = GoalStatusEnum.getEnumByExternalCode(csvWearableFeedbackDTO.getGoalStatus());
        }
        WearableFeedbackEntity wearableFeedbackEntity = new WearableFeedbackEntity(
                null,
                csvWearableFeedbackDTO.getWearableCode(),
                csvWearableFeedbackDTO.getPosX(),
                csvWearableFeedbackDTO.getPosY(),
                csvWearableFeedbackDTO.getHeartRatePerMinute(),
                csvWearableFeedbackDTO.getBodyTemperature(),
                csvWearableFeedbackDTO.getOxygenation(),
                goalStatus,
                triggerTime.atZone(ZoneId.systemDefault()).toInstant().toEpochMilli(),
                triggerTime,
                wearableEntity);
        wearableFeedbackRepository.save(wearableFeedbackEntity);

        // TODO: think how to join goal with node
    }

}
