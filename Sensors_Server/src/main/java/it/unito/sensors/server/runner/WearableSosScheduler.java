package it.unito.sensors.server.runner;

import it.unito.sensors.server.service.WearableSosFileHandler;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

/**
 * \brief WearableSosScheduler polls every X milliseconds the wearable sos folder of the storage.
 */
@Component
@EnableScheduling
public class WearableSosScheduler {

    @Autowired
    WearableSosFileHandler wearableSosFileHandler;

    @Scheduled(fixedRate= 100 , initialDelay = 1000)
    public void run(){
        wearableSosFileHandler.watchFolder();
    }

}
