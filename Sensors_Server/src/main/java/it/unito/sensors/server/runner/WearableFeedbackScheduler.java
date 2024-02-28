package it.unito.sensors.server.runner;

import it.unito.sensors.server.service.WearableFeedbackFileHandler;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

/**
 * \brief WearableFeedbackScheduler polls every X milliseconds the wearable feedback folder of the storage.
 */
@Component
@EnableScheduling
public class WearableFeedbackScheduler {

    @Autowired
    WearableFeedbackFileHandler wearableFeedbackFileHandler;

    @Scheduled(fixedRate= 100 , initialDelay = 1000)
    public void run(){
        wearableFeedbackFileHandler.watchFolder();
    }

}
