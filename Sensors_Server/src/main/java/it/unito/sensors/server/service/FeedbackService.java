package it.unito.sensors.server.service;

import it.unito.sensors.server.configuration.AppConfiguration;
import it.unito.sensors.server.entity.FeedbackEntity;
import it.unito.sensors.server.entity.GoalStatusEnum;
import it.unito.sensors.server.entity.NodeEntity;
import it.unito.sensors.server.fs.dto.CSVFeedbackDTO;
import it.unito.sensors.server.repository.FeedbackRepository;
import it.unito.sensors.server.repository.NodeRepository;
import lombok.extern.slf4j.Slf4j;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.util.TimeZone;

/**
 * \brief FeedbackService exposes to other services some extended features about actions.
 */
@Service
@Slf4j
public class FeedbackService {

    @Autowired
    AppConfiguration appConfiguration;

    @Autowired
    FeedbackRepository feedbackRepository;

    @Autowired
    NodeRepository nodeRepository;

    @Autowired
    GoalService goalService;

    public FeedbackService(AppConfiguration appConfiguration) {
        this.appConfiguration = appConfiguration;
    }

    /**
     * Save a new feedback and update goal status, if it's different from ABORTED (final state) and NO_GOAL (undefined state)
     * @param cSVFeedbackDTO
     */
    public void joinWithNodeAndPersist(CSVFeedbackDTO cSVFeedbackDTO) {
        NodeEntity nodeEntity = nodeRepository.findByName(cSVFeedbackDTO.getRobotName());
        if (nodeEntity == null) {
            log.warn("request to join with an unknown node: {}", cSVFeedbackDTO.getRobotName());
            return;
        }

        FeedbackEntity lastFeedback = feedbackRepository.findTop1ByNodeEntityOrderByUnixTimestampDesc(nodeEntity);
        if (lastFeedback != null) {
//            log.info("request to persist feedback: last feedback {} vs new feedback {}", lastFeedback.getUnixTimestamp(), cSVFeedbackDTO.getUnixTimestamp());
            if (lastFeedback.getUnixTimestamp() > cSVFeedbackDTO.getUnixTimestamp()) {
                log.warn("request to persist an old feedback: last feedback {} > new feedback {}", lastFeedback.getUnixTimestamp(), cSVFeedbackDTO.getUnixTimestamp());
            }
        }

        LocalDateTime triggerTime = LocalDateTime.now();
        if (cSVFeedbackDTO.getUnixTimestamp() != null) {
            triggerTime = LocalDateTime.ofInstant(
                    Instant.ofEpochMilli(cSVFeedbackDTO.getUnixTimestamp()),
                    TimeZone.getDefault().toZoneId()
            );
        }

        GoalStatusEnum goalStatus = GoalStatusEnum.NO_GOAL;
        if (cSVFeedbackDTO.getGoalStatus() >= 0) {
            goalStatus = GoalStatusEnum.getEnumByExternalCode(cSVFeedbackDTO.getGoalStatus());
        }
        FeedbackEntity feedbackEntity = new FeedbackEntity(
                null,
                cSVFeedbackDTO.getRobotName(),
                cSVFeedbackDTO.getPosX(),
                cSVFeedbackDTO.getPosY(),
                cSVFeedbackDTO.getOrientW(),
                cSVFeedbackDTO.getBatteryChargePercentage(),
                goalStatus,
                triggerTime.atZone(ZoneId.systemDefault()).toInstant().toEpochMilli(),
                triggerTime,
                nodeEntity);
        feedbackRepository.save(feedbackEntity);

        // update to ABORT status only via goal.action=ABORT
        if (StringUtils.isNotEmpty(cSVFeedbackDTO.getGoalCode()) &&
                goalStatus != GoalStatusEnum.NO_GOAL &&
                goalStatus != GoalStatusEnum.ABORTED) {
            goalService.joinGoalWithNode(cSVFeedbackDTO.getGoalCode(), nodeEntity.getName(), goalStatus, triggerTime, true);
        }
    }

}
