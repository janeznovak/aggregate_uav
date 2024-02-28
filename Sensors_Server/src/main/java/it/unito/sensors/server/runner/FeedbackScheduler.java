package it.unito.sensors.server.runner;

import it.unito.sensors.server.service.FeedbackFileHandler;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

/**
 * \brief FeedbackScheduler polls every X milliseconds the feedback folder of the storage.
 */
@Component
@EnableScheduling
public class FeedbackScheduler {

    @Autowired
    FeedbackFileHandler feedbackFileHandler;

    @Scheduled(fixedRate= 100 , initialDelay = 1000)
    public void run(){
        feedbackFileHandler.watchFolder();
    }

}
