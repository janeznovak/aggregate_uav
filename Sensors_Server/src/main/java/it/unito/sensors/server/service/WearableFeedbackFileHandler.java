package it.unito.sensors.server.service;

import it.unito.sensors.server.configuration.AppConfiguration;
import it.unito.sensors.server.fs.Reader;
import it.unito.sensors.server.fs.dto.CSVWearableFeedbackDTO;
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
 * \brief WearableFeedbackFileHandler polls and handles wearable feedbacks files.
 */
@Service
@Slf4j
public class WearableFeedbackFileHandler extends FileHandlerService<CSVWearableFeedbackDTO, Reader<CSVWearableFeedbackDTO>> implements IWatcher {

    @Autowired
    WearableFeedbackService wearableFeedbackService;

    public WearableFeedbackFileHandler(AppConfiguration appConfiguration,
                                       Reader<CSVWearableFeedbackDTO> reader,
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
            String pathFolder = FsUtil.getWearablePathFolder(appConfiguration.getWearableFeedback().getPathToRead(), appConfiguration, wearableCode);
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
     * Parse wearable feedback and create new one.
     * @param wearableFeedbackDTOReader
     * @param filePath
     */
    @Override
    public void handleFile(Reader<CSVWearableFeedbackDTO> wearableFeedbackDTOReader, String filePath) {
            List<CSVWearableFeedbackDTO> dtos = wearableFeedbackDTOReader.read(filePath);
            if (dtos == null || dtos.isEmpty()) {
                log.error("Wearable feedback {} file is empty", filePath);
                return;
            }
            CSVWearableFeedbackDTO csvWearableFeedbackDTO = dtos.get(0);
            log.info("Read wearable feedback {}", csvWearableFeedbackDTO);

            // join feedback with node and persist
            wearableFeedbackService.saveNewFeedback(csvWearableFeedbackDTO);
    }
}
