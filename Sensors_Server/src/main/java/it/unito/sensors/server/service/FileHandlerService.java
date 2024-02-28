package it.unito.sensors.server.service;

import it.unito.sensors.server.configuration.AppConfiguration;
import it.unito.sensors.server.fs.Reader;
import it.unito.sensors.server.fs.util.FsUtil;
import it.unito.sensors.server.util.IWatcher;
import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.extern.slf4j.Slf4j;

import java.io.File;
import java.io.IOException;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.List;

@Slf4j
/**
 * \brief Abstract class with common implementation to poll and parse files.
 */
public abstract class FileHandlerService<DTO, R extends Reader<DTO>> implements IWatcher {

    AppConfiguration appConfiguration;

    Reader<DTO> reader;

    List<PathWatcher> pathWatchers;

    WatchService watchService;

    public FileHandlerService(AppConfiguration appConfiguration,
                              Reader<DTO> reader) {
        this.appConfiguration = appConfiguration;
        this.reader = reader;
        this.pathWatchers = new ArrayList<>();

        // STEP1: Create a watch service
        try {
            watchService = FileSystems.getDefault().newWatchService();
        } catch (IOException e) {
            log.error("Failed configuring reader");
        }

        init(appConfiguration);
    }

    /**
     * Get the path of the directory which you want to monitor.
     * Register the directory with the watch service
     * @param appConfiguration
     */
    abstract void init(AppConfiguration appConfiguration);

    /**
     * Parse file and handle content
     * @param reader
     * @param filePath
     */
    abstract void handleFile(Reader<DTO> reader, String filePath);

    @Override
    public void watchFolder() {
        for (PathWatcher pathWatcher : pathWatchers) {
            try {
                for (WatchEvent<?> event : pathWatcher.watchKey.pollEvents()) {
                    // STEP5: Get file name from even context
                    WatchEvent<Path> pathEvent = (WatchEvent<Path>) event;

                    Path fileName = pathEvent.context();

                    // TODO: add regex to abstract method
                    String regex = ".*\\.txt$";
                    if (fileName.toString().matches(regex)) {
                        // STEP6: Check type of event.
                        WatchEvent.Kind<?> kind = event.kind();

                        // STEP7: Perform necessary action with the event
                        if (kind == StandardWatchEventKinds.ENTRY_CREATE) {
                            log.debug("A new file is created : " + fileName);
                            String filePath = pathWatcher.getPath() + fileName;

                            handleFile(reader, filePath);

                            // delete file acquired
                            File file = new File(filePath);
                            file.delete();
                        }
                    }
                }

                // STEP8: Reset the watch key everytime for continuing to use it for further event polling
                pathWatcher.watchKey.reset();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }


    @Getter
    @AllArgsConstructor
    class PathWatcher {
        protected String path;
        protected WatchKey watchKey;
    }
}
