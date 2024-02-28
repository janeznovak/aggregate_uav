package it.unito.sensors.server.service;

import it.unito.sensors.server.configuration.AppConfiguration;
import it.unito.sensors.server.dto.GoalDTO;
import it.unito.sensors.server.entity.ActionStateEnum;
import it.unito.sensors.server.entity.GoalEntity;
import it.unito.sensors.server.entity.GoalStatusEnum;
import it.unito.sensors.server.entity.GoalSubCodeEnum;
import it.unito.sensors.server.fs.Reader;
import it.unito.sensors.server.fs.dto.CSVWearableSosDTO;
import it.unito.sensors.server.fs.util.FsUtil;
import it.unito.sensors.server.util.IWatcher;
import lombok.extern.slf4j.Slf4j;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.StandardWatchEventKinds;
import java.time.Duration;
import java.time.Instant;
import java.time.LocalDateTime;
import java.util.List;
import java.util.Random;
import java.util.TimeZone;


/**
 * \brief WearableSosFileHandler polls and handles wearable sos files.
 */
@Service
@Slf4j
public class WearableSosFileHandler extends FileHandlerService<CSVWearableSosDTO, Reader<CSVWearableSosDTO>> implements IWatcher {

    @Autowired
    WearableFeedbackService wearableFeedbackService;

    @Autowired
    GoalService goalService;

    public WearableSosFileHandler(AppConfiguration appConfiguration,
                                  Reader<CSVWearableSosDTO> reader,
                                  WearableFeedbackService wearableFeedbackService) {
        super(appConfiguration, reader);
        this.wearableFeedbackService = wearableFeedbackService;
    }

    /**
     * Initialize correctly the polling in the wearable folder.
     * @param appConfiguration
     */
    @Override
    void init(AppConfiguration appConfiguration) {
        for (String wearableCode : appConfiguration.getWearableCodes()) {
            // STEP2: Get the path of the directory which you want to monitor.
            String pathFolder = FsUtil.getWearablePathFolder(appConfiguration.getWearableSos().getPathToRead(), appConfiguration, wearableCode);
            Path directory = Path.of(pathFolder);
            log.info("Watching directory {} for changes", pathFolder);

            // create folder if not exists
            if (!directory.toFile().exists()) directory.toFile().mkdirs();

            // STEP3: Register the directory with the watch service
            try {
                pathWatchers.add(new PathWatcher(pathFolder, directory.register(watchService, StandardWatchEventKinds.ENTRY_CREATE)));
            } catch (IOException e) {
                log.error("Failed register watchkey");
            }
        }
    }

    /**
     * Parse new sos from feedback and create new goal
     * @param csvWearableSosDTOReader
     * @param filePath
     */
    @Override
    public void handleFile(Reader<CSVWearableSosDTO> csvWearableSosDTOReader, String filePath) {
        List<CSVWearableSosDTO> dtos = csvWearableSosDTOReader.read(filePath);
        if (dtos == null || dtos.isEmpty()) {
            log.error("Wearable sos {} file is empty", filePath);
            return;
        }
        CSVWearableSosDTO csvWearableSosDTO = dtos.get(0);
        log.info("Read wearable sos {}", csvWearableSosDTO);

        Integer timeoutAfterLastCompletion = appConfiguration.getGoalStateMachine().getTimeout().get(GoalStatusEnum.REACHED.getCode());

        if (!isTriggered(csvWearableSosDTO.getFollowTriggered())) {

            GoalEntity existingRunningSOS = goalService.findExistingRunningSOSGoalIfExists(csvWearableSosDTO.getWearableCode());
            if (existingRunningSOS == null) {
                GoalEntity lastCompletedSOS = goalService.findLastCompletedSOSGoalIfExists(csvWearableSosDTO.getWearableCode());
                if (
                    lastCompletedSOS == null ||
                    timeoutAfterLastCompletion == null ||
                    LocalDateTime.now().isAfter(lastCompletedSOS.getTimestampUpdate().plus(Duration.ofSeconds(timeoutAfterLastCompletion)))
                ) {
                    // read target position from last feedback received from wearable
                    GoalDTO goalDTO = GoalDTO.builder()
                            .goalCode(String.format("SOS-%s", System.currentTimeMillis()))
                            .action(ActionStateEnum.SOS.getCode())
                            .posX(csvWearableSosDTO.getPosX()-getOffset()) // compute interval x position
                            .posY(csvWearableSosDTO.getPosY()-getOffset()) // compute interval y position
                            .orientW(1.0f) //default
                            .referenceCode(csvWearableSosDTO.getWearableCode())
                            .priority(csvWearableSosDTO.getPriority())
                            .subCode(getSubCode(csvWearableSosDTO))
                            .description(getDescription(csvWearableSosDTO))
                            .timestampCreation(LocalDateTime.now())
                            .build();
                    goalService.createGoal(goalDTO);
                } else {
                    log.info("Discarded goal for {}", csvWearableSosDTO.getWearableCode());
                }
            } else {
                log.info("Already exists a goal for {}", csvWearableSosDTO.getWearableCode());
                // TODO: at the moment we don't update existing goal
            }
        } else {
            // TODO: at the moment follow action is not handled
        }
    }

    // TODO: use labels
    /**
     * Returns a description considering the properties of incoming wearables SOS
     * @param csvWearableSosDTO
     * @return
     */
    private String getDescription(CSVWearableSosDTO csvWearableSosDTO) {
        StringBuilder stringBuilder = new StringBuilder();
        if (isTriggered(csvWearableSosDTO.getAccidentalFallTriggered())) {
            stringBuilder.append("caduta accidentale: si; ");
        }
        if (isTriggered(csvWearableSosDTO.getHeartRatePerMinuteTriggered())) {
            stringBuilder.append(String.format("ultimo battito cardiaco rilevato: %s; ", csvWearableSosDTO.getHeartRatePerMinute()));
        }
        if (isTriggered(csvWearableSosDTO.getOxygenationTriggered())) {
            stringBuilder.append(String.format("ultima ossigenazione rilevata: %s; ", csvWearableSosDTO.getOxygenation()));
        }
        if (isTriggered(csvWearableSosDTO.getBodyTemperatureTriggered())) {
            stringBuilder.append(String.format("ultima temperatura rilevata: %s; ", csvWearableSosDTO.getBodyTemperature()));
        }
        if (StringUtils.isEmpty(stringBuilder.toString())) {
            stringBuilder.append("chiamata manuale di aiuto");
        }
        return stringBuilder.toString();
    }

    /**
     * Returns the specific enum of GoalSubCodeEnum in according to the incoming wearables SOS
     * @param csvWearableSosDTO
     * @return
     */
    private GoalSubCodeEnum getSubCode(CSVWearableSosDTO csvWearableSosDTO) {
        if (isTriggered(csvWearableSosDTO.getAccidentalFallTriggered())) {
            return GoalSubCodeEnum.ACCIDENTAL_FALL;
        } else if (isTriggered(csvWearableSosDTO.getHeartRatePerMinuteTriggered())) {
            return GoalSubCodeEnum.HEART;
        } else if (isTriggered(csvWearableSosDTO.getOxygenationTriggered())) {
            return GoalSubCodeEnum.OXYGENATION;
        } else if (isTriggered(csvWearableSosDTO.getBodyTemperatureTriggered())) {
            return GoalSubCodeEnum.BODY_TEMPERATURE;
        } else if (isTriggered(csvWearableSosDTO.getFollowTriggered())) {
            return GoalSubCodeEnum.FOLLOW;
        }
        return GoalSubCodeEnum.NONE;
    }

    /**
     * Convert a string rappresentation of boolean to boolean
     * @param booleanString
     * @return
     */
    private Boolean isTriggered(String booleanString) {
        return Boolean.parseBoolean(booleanString);
    }

    /**
     * Generate a number between 0.25 and 0.5 to used as an offset of a goal
     * @param booleanString
     * @return
     */
    private Float getOffset() {
        Random random = new Random();
        float min = 0.25f;
        float max = 0.5f;
        return min + (max - min) * random.nextFloat();
    }
}
