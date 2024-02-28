package it.unito.sensors.server.service;

import it.unito.sensors.server.configuration.AppConfiguration;
import it.unito.sensors.server.entity.ActionStateEnum;
import it.unito.sensors.server.entity.GoalStatusEnum;
import it.unito.sensors.server.fs.Reader;
import it.unito.sensors.server.fs.dto.CSVActionDTO;
import it.unito.sensors.server.fs.util.FsUtil;
import it.unito.sensors.server.util.IWatcher;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.StandardWatchEventKinds;
import java.time.Instant;
import java.time.LocalDateTime;
import java.util.List;
import java.util.TimeZone;

/**
 * \brief ActionFileHandler polls and handles actions files.
 */
@Slf4j
@Service
public class ActionFileHandler extends FileHandlerService<CSVActionDTO, Reader<CSVActionDTO>> implements IWatcher {

    GoalService goalService;

    public ActionFileHandler(AppConfiguration appConfiguration,
                             Reader<CSVActionDTO> reader,
                             GoalService goalService) {
        super(appConfiguration, reader);
        this.goalService = goalService;
    }

    /**
     * Initialize correctly the polling in the robot's folder.
     *
     * @param appConfiguration
     */
    @Override
    public void init(AppConfiguration appConfiguration) {
        for (String robotName : appConfiguration.getRobotNames()) {
            // STEP2: Get the path of the directory which you want to monitor.
            String pathFolder = FsUtil.getRobotPathFolder(appConfiguration.getAction().getPathToRead(), appConfiguration, robotName);
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
     * Watch action folder, updating goal status, according to the coming file.
     * - if action is GOAL, will be saved status=NO_GOAL (because at the moment we don't have any feedback updates)
     * - if action is ABORT, will be saved status=ABORTED
     */
    @Override
    public void handleFile(Reader<CSVActionDTO> actionReader, String filePath) {
        List<CSVActionDTO> dtos = actionReader.read(filePath);
        if (dtos == null || dtos.isEmpty()) {
            log.error("Action {} file is empty", filePath);
            return;
        }
        CSVActionDTO csvActionDTO = dtos.get(0);
        log.info("Read action {}", csvActionDTO);

        // join goal with node
        ActionStateEnum actionStateEnum = ActionStateEnum.valueOf((csvActionDTO.getAction()));
        LocalDateTime triggerTime = LocalDateTime.now();
        if (csvActionDTO.getUnixTimestamp() != null) {
            triggerTime = LocalDateTime.ofInstant(
                    Instant.ofEpochMilli(csvActionDTO.getUnixTimestamp()),
                    TimeZone.getDefault().toZoneId()
            );
        }
        if (ActionStateEnum.GOAL == actionStateEnum) {
            goalService.joinGoalWithNode(csvActionDTO.getGoal(), csvActionDTO.getRobot(), GoalStatusEnum.NO_GOAL, triggerTime);
        } else if (ActionStateEnum.ABORT == actionStateEnum) {
            goalService.joinGoalWithNode(csvActionDTO.getGoal(), csvActionDTO.getRobot(), GoalStatusEnum.ABORTED, triggerTime);
        } else if (ActionStateEnum.SOS == actionStateEnum) {
            goalService.joinGoalWithNode(csvActionDTO.getGoal(), csvActionDTO.getRobot(), GoalStatusEnum.NO_GOAL, triggerTime);
        }
    }

}

