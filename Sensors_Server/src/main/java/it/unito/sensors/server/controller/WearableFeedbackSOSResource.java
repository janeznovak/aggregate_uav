package it.unito.sensors.server.controller;

import it.unito.sensors.server.dto.CustomWearableSOSDTO;
import it.unito.sensors.server.dto.GoalDTO;
import it.unito.sensors.server.util.BashScriptRunner;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RestController;

import java.io.IOException;
import java.util.stream.Collectors;

/**
 * \brief REST API to handle generate custom feedback from wearable.
 */
@RestController
@Slf4j
public class WearableFeedbackSOSResource {

    @Autowired
    BashScriptRunner bashScriptRunner;

    @PostMapping(
            value = "/wearable-sos",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public ResponseEntity<Boolean> createCustomWearableSOS(@RequestBody CustomWearableSOSDTO customWearableSOSDTO) throws IOException, InterruptedException {
        bashScriptRunner.runBashScript("trigger_sos_event.sh", customWearableSOSDTO.getWearableCode(), customWearableSOSDTO.getTriggerEvent());
        return ResponseEntity.ok(true);
    }

}
