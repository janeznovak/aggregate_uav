package it.unito.sensors.server.service;

import it.unito.sensors.server.configuration.AppConfiguration;
import it.unito.sensors.server.configuration.dto.CallbackConfigProperties;
import it.unito.sensors.server.dto.GoalDTO;
import it.unito.sensors.server.entity.*;
import it.unito.sensors.server.fs.GoalWriter;
import it.unito.sensors.server.fs.dto.FilePersistedDTO;
import it.unito.sensors.server.repository.GoalRepository;
import it.unito.sensors.server.repository.ItemRepository;
import it.unito.sensors.server.repository.NodeRepository;
import it.unito.sensors.server.util.BashScriptRunner;
import lombok.extern.slf4j.Slf4j;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import javax.transaction.Transactional;
import java.time.LocalDateTime;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;

import static it.unito.sensors.server.configuration.dto.CallbackConfigProperties.BASH_TYPE;


/**
 * \brief GoalService exposes to other services some extended features about goals.
 */
@Slf4j
@Service
public class GoalService {

    GoalWriter goalWriter;

    GoalRepository goalRepository;

    ItemRepository itemRepository;

    NodeRepository nodeRepository;

    AppConfiguration appConfiguration;

    BashScriptRunner bashScriptRunner;

    public GoalService(GoalWriter goalWriter,
                       GoalRepository goalRepository,
                       ItemRepository itemRepository,
                       NodeRepository nodeRepository,
                       BashScriptRunner bashScriptRunner,
                       AppConfiguration appConfiguration) {
        this.goalWriter = goalWriter;
        this.goalRepository = goalRepository;
        this.itemRepository = itemRepository;
        this.nodeRepository = nodeRepository;
        this.bashScriptRunner = bashScriptRunner;
        this.appConfiguration = appConfiguration;
    }

    @Transactional
    public boolean createGoal(GoalDTO goalDTO) {
        return createGoal(goalDTO, true);
    }
    /**
     * Create a goal in a database transaction, sending it to the robots via file.
     * @param goalDTO
     * @return
     */
    @Transactional
    public boolean createGoal(GoalDTO goalDTO, Boolean sendToRobots) {
        if (ActionStateEnum.GOAL.getCode().equals(goalDTO.getAction())) {
            log.info("creating new goal '{}' with values: {}", goalDTO.getGoalCode(), goalDTO);

            log.info("searching item coordinates for '{}'", goalDTO.getReferenceCode());
            Optional<ItemEntity> itemEntity = searchItem(goalDTO);
            if (itemEntity.isPresent()) {
                log.info("item '{}' found: {}", goalDTO.getReferenceCode(), itemEntity.get());
                goalDTO.setPosX(itemEntity.get().getPosX());
                goalDTO.setPosY(itemEntity.get().getPosY());
                goalDTO.setOrientW(itemEntity.get().getOrientW());
                goalDTO.setReferenceCode(itemEntity.get().getCode());
                goalDTO.setSubCode(GoalSubCodeEnum.NONE);
                goalDTO.setSource("UNKNOWN");

                // create goal
                GoalEntity goalEntity = createOrUpdateGoal(goalDTO);

                // send goal to robots (via file)
                goalDTO.setTimestampCreation(goalEntity.getTimestampCreation());
                if (sendToRobots) {
                    sendGoalToToRobots(goalDTO);
                }

                return true;
            } else {
                log.error("item '{}' not found", goalDTO.getReferenceCode());
                return false;
            }
        } else if (ActionStateEnum.ABORT.getCode().equals(goalDTO.getAction())) {
            log.info("aborting previous goal '{}'", goalDTO.getGoalCode());

            // set ABORTED status
            goalDTO.setStatus(GoalStatusEnum.ABORTED);
            goalDTO.setSubCode(null);
            // lock goal: in this way, current goal will not be updated from other nodes
            goalDTO.setLock(true);
            GoalEntity goalEntity = createOrUpdateGoal(goalDTO);

            // send goal to robots (via file)
            goalDTO.setTimestampCreation(goalEntity.getTimestampCreation());
            if (sendToRobots) {
                sendGoalToToRobots(goalDTO);
            }

            return true;
        } else if (ActionStateEnum.SOS.getCode().equals(goalDTO.getAction())) {
            log.info("creating new goal or existing '{}' with values: {}", goalDTO.getGoalCode(), goalDTO);

            // create goal
            goalDTO.setStatus(GoalStatusEnum.NO_GOAL);
            goalDTO.setSource(goalDTO.getReferenceCode());
            GoalEntity goalEntity = createOrUpdateGoal(goalDTO);

            // send goal to robots (via file)
            goalDTO.setTimestampCreation(goalEntity.getTimestampCreation());
            if (sendToRobots) {
                sendGoalToToRobots(goalDTO);
            }

            return true;
        } else if (ActionStateEnum.FOLLOW.getCode().equals(goalDTO.getAction())) {
            // TODO:
        }

        log.info("action '{}' not managed at the moment", goalDTO.getAction());

        return false;
    }

    public boolean joinGoalWithNode(String goalCode, String nodeName, GoalStatusEnum goalStatusEnum, LocalDateTime localDateTime) {
        return joinGoalWithNode(goalCode, nodeName, goalStatusEnum, localDateTime, false);
    }

    /**
     * Update the status of goal that currently a robot runs, finding by goal code.
     * On demand, the method can add some checks in order to change the status congruently with own state machine.
     * @param nodeName
     * @param goalStatusEnum
     * @return
     */
    public boolean joinGoalWithNode(String goalCode, String nodeName, GoalStatusEnum goalStatusEnum, LocalDateTime localDateTime, Boolean check) {
        NodeEntity nodeEntity = nodeRepository.findByName(nodeName);
        if (nodeEntity == null) {
            log.warn("request to join with an unknown node: {}", nodeName);
            return false;
        }

        GoalEntity goal = goalRepository.findByCode(goalCode);
        if (goal == null) {
            log.warn("request to join an unknown {} goal of node: {}", goalCode, nodeName);
            return false;
        }

        // if goal is lock, skip
        if (goal.getLock() != null && goal.getLock()){
            log.info("request to join a locked goal");
            return false;
        }

        if (check) {
            if (goal.getNodeEntity() == null || !goal.getNodeEntity().getId().equals(nodeEntity.getId())) {
                log.warn("request to join a goal with code {} not joined with node: {}", goalCode, nodeName);
                return false;
            }

            // if previous status is REACHED, skip
            if (GoalStatusEnum.REACHED == goal.getStatus()) {
                log.debug("request to join an already reached goal of node: {}", nodeName);
                return false;
            }

            // if a reached status is requested, and previous status was not RUNNING, skip
            if (GoalStatusEnum.REACHED == goalStatusEnum && GoalStatusEnum.RUNNING != goal.getStatus()){
                log.info("request to join a reached goal without previously to be set to RUNNING");
                return false;
            }

            // if a reached status is requested, and node are different, skip
            if (GoalStatusEnum.REACHED == goalStatusEnum && !nodeEntity.getName().equals(goal.getNodeEntity().getName())){
                log.info("request to join a reached goal with a different node");
                return false;
            }

            //        if (goalStatusEnum == goal.getStatus()) {
            //            log.info("request to join a goal because it's like previous: {}", goal.getStatus());
            //            return false;
            //        }
        }

        log.debug("request to join goal {} with status {}, with node {}, previous node {}, lock {}", goal.getCode(), goalStatusEnum.getCode(), nodeName, nodeEntity.getName(), goal.getLock());

        // update timestamp
        goal.setTimestampUpdate(localDateTime);
        // update status
        goal.setStatus(goalStatusEnum);
        // abort can't change the owner of goal
        if (GoalStatusEnum.ABORTED != goalStatusEnum) {
            goal.setNodeEntity(nodeEntity);
        }

        // update goal with node
        goalRepository.save(goal);

        // find if there are some callbacks for goal status
        if (appConfiguration.getGoalStateMachine().getCallback().containsKey(goal.getStatus().getCode())) {
            CallbackConfigProperties callbackConfigProperties = appConfiguration.getGoalStateMachine().getCallback().get(goal.getStatus().getCode());

            log.debug("Possible callbacks found: type:{}, name:{}, goal_actions:{}", callbackConfigProperties.getType(), callbackConfigProperties.getName(), callbackConfigProperties.getGoalActions());

            try {
                if (BASH_TYPE.equals(callbackConfigProperties.getType()) &&
                        StringUtils.isNotEmpty(callbackConfigProperties.getName()) &&
                        callbackConfigProperties.getGoalActions().contains(goal.getAction())
                ) {
                    log.info("Run callback: type:{}, name:{}", callbackConfigProperties.getType(), callbackConfigProperties.getName());
                    CompletableFuture<String> runBashScriptAsyncC = bashScriptRunner.runBashScriptAsync(callbackConfigProperties.getName(), goal.getNodeEntity().getName(), goal.getSource());
                    runBashScriptAsyncC.thenAccept(result -> log.info("Script result: {}", result));
                }
            } catch (Exception e) {
                log.error("Error calling callback: ",e);
            }
        }

        return true;
    }

    /**
     * Send a goalDTO to robots, saving file in the Storage
     * @param goalDTO
     */
    private void sendGoalToToRobots(GoalDTO goalDTO) {
        // write to file
        Optional<FilePersistedDTO> persisted = goalWriter.write(goalDTO);
    }

    GoalEntity findExistingRunningSOSGoalIfExists(String wearableCode) {
        return goalRepository.findByStatusNotAndReferenceCodeAndAction(GoalStatusEnum.REACHED, wearableCode, ActionStateEnum.SOS.getCode());
    }

    GoalEntity findLastCompletedSOSGoalIfExists(String wearableCode) {
        return goalRepository.findTop1ByStatusAndReferenceCodeAndActionOrderByTimestampUpdateDesc(GoalStatusEnum.REACHED, wearableCode, ActionStateEnum.SOS.getCode());
    }

    /**
     * Search an item entity by item.code
     * @param goalDTO
     * @return
     */
    public Optional<ItemEntity> searchItem(GoalDTO goalDTO) {
        return Optional.ofNullable(itemRepository.findByCode(goalDTO.getReferenceCode()));
    }

    /**
     * Create or update goal:
     * - create a new goal with data coming from item to be find
     * - update status, action, locked status of an existing goal
     * @param goalDTO
     */
    private GoalEntity createOrUpdateGoal(GoalDTO goalDTO) {
        GoalEntity goalFound = goalRepository.findByCode(goalDTO.getGoalCode());
        LocalDateTime now = LocalDateTime.now();
        if (goalFound != null) {
            // update action
            goalFound.setAction((goalDTO.getAction() != null) ? goalDTO.getAction() : goalFound.getAction());
            goalFound.setStatus((goalDTO.getStatus() != null) ? goalDTO.getStatus() : goalFound.getStatus());
            goalFound.setSubCode((goalDTO.getSubCode() != null) ? goalDTO.getSubCode() : goalFound.getSubCode());
            goalFound.setPriority((goalDTO.getPriority() != null) ? goalDTO.getPriority() : goalFound.getPriority());
            goalFound.setDescription((goalDTO.getDescription() != null) ? goalDTO.getDescription() : goalFound.getDescription());
            goalFound.setLock(goalDTO.getLock());
            goalFound.setTimestampUpdate(now);
            return goalRepository.save(goalFound);
        } else {
            // new goal
            GoalEntity goalEntity = GoalEntity.builder()
                                        .action(goalDTO.getAction())
                                        .code(goalDTO.getGoalCode())
                                        .description(goalDTO.getDescription())
                                        .status(goalDTO.getStatus())
                                        .posX(goalDTO.getPosX())
                                        .posY(goalDTO.getPosY())
                                        .orientW(goalDTO.getOrientW())
                                        .priority(goalDTO.getPriority())
                                        .subCode(goalDTO.getSubCode())
                                        .referenceCode(goalDTO.getReferenceCode())
                                        .source(goalDTO.getSource())
                                        .timestampCreation(now)
                                        .build();
            return goalRepository.save(goalEntity);
        }
    }

}
