package it.unito.sensors.server.service;

import it.unito.sensors.server.configuration.AppConfiguration;
import it.unito.sensors.server.fs.Reader;
import it.unito.sensors.server.fs.dto.CSVFeedbackDTO;
import it.unito.sensors.server.fs.util.FsUtil;
import it.unito.sensors.server.util.IWatcher;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.StandardWatchEventKinds;
import java.util.List;

/**
 * \brief FeedbackFileHandler polls and handles feedbacks files.
 */
@Service
@Slf4j
public class FeedbackFileHandler extends FileHandlerService<CSVFeedbackDTO, Reader<CSVFeedbackDTO>> implements IWatcher {

    @Autowired
    FeedbackService feedbackService;

    public FeedbackFileHandler(AppConfiguration appConfiguration,
                               Reader<CSVFeedbackDTO> reader,
                               FeedbackService feedbackService) {
        super(appConfiguration, reader);
        this.feedbackService = feedbackService;
    }

    /**
     * Initialize correctly the polling in the robot's folder.
     * @param appConfiguration
     */
    @Override
    void init(AppConfiguration appConfiguration) {
        for (String robotName : appConfiguration.getRobotNames()) {
            // STEP2: Get the path of the directory which you want to monitor.
            String pathFolder = FsUtil.getRobotPathFolder(appConfiguration.getFeedback().getPathToRead(), appConfiguration, robotName);
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

    @Override
    public void handleFile(Reader<CSVFeedbackDTO> feedbackReader, String filePath) {
            List<CSVFeedbackDTO> dtos = feedbackReader.read(filePath);
            if (dtos == null || dtos.isEmpty()) {
                log.error("Feedback {} file is empty", filePath);
                return;
            }
            CSVFeedbackDTO csvFeedbackDTO = dtos.get(0);
            log.info("Read feedback {}", csvFeedbackDTO);

            // join feedback with node and persist
            feedbackService.joinWithNodeAndPersist(csvFeedbackDTO);
    }
}
