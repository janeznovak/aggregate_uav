package it.unito.sensors.server.fs;

import it.unito.sensors.server.configuration.AppConfiguration;
import it.unito.sensors.server.dto.GoalDTO;
import it.unito.sensors.server.fs.dto.FilePersistedDTO;
import it.unito.sensors.server.fs.util.CSVWriterUtil;
import it.unito.sensors.server.service.StorageService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.io.File;
import java.time.ZoneId;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * \brief GoalWriter sends to the storage a list of GoalDTO, writing in a file.
 */
@Service
@Slf4j
public class GoalWriter implements Writer<GoalDTO> {

    @Autowired
    AppConfiguration appConfiguration;

    @Autowired
    CSVWriterUtil csvWriterUtil;

    @Autowired
    StorageService storageService;

    @Override
    public Optional<FilePersistedDTO> write(GoalDTO goalDTO) {
        try {
            // create folder if not exists
            File dir = new File(appConfiguration.getGoal().getPathToWrite());
            if (!dir.exists()) dir.mkdirs();

            List<String[]> dataLines = new ArrayList<>();
            dataLines.add(new String[]
                    {
                            goalDTO.getAction(),
                            goalDTO.getGoalCode(),
                            (goalDTO.getPosX() != null) ? goalDTO.getPosX().toString() : "",
                            (goalDTO.getPosY() != null) ? goalDTO.getPosY().toString() : "",
                            (goalDTO.getOrientW() != null) ? goalDTO.getOrientW().toString() : "",
                            goalDTO.getSource(),
                            (goalDTO.getPriority() != null) ? goalDTO.getPriority().toString() : "",
                            (goalDTO.getSubCode() != null) ? goalDTO.getSubCode().getCode() : "",
                            String.valueOf(goalDTO.getTimestampCreation().atZone(ZoneId.systemDefault()).toInstant().toEpochMilli())
                    }
            );
            String absoluteFile = appConfiguration.getGoal().getPathToWrite() + getFileName(goalDTO);
            log.info("writing file: {}", absoluteFile);

            byte[] data = csvWriterUtil.writeToFile(absoluteFile, dataLines);

            String pathWithStorageAsBasePath = storageService.convertToStoragePath(absoluteFile, true);

            return Optional.of(
                    FilePersistedDTO.builder()
                    .path(pathWithStorageAsBasePath)
                    .absolutePath(absoluteFile)
                    .data(data)
                    .build()
            );
        } catch (Exception e) {
            log.error("Error writing file", e);
            return Optional.empty();
        }
    }

    private String getFileName(GoalDTO goalDTO) {
        return String.format("new-%s-%s.txt", goalDTO.getAction(), System.currentTimeMillis());
    }

}
