package it.unito.sensors.server.runner;

import it.unito.sensors.server.service.ActionFileHandler;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

/**
 * \brief ActionScheduler polls every X milliseconds the action folder of the storage.
 */
@Component
@EnableScheduling
public class ActionScheduler {

    @Autowired
    ActionFileHandler actionFileHandler;

    @Scheduled(fixedRate= 100 , initialDelay = 1000)
    public void run(){
        actionFileHandler.watchFolder();
    }

}
