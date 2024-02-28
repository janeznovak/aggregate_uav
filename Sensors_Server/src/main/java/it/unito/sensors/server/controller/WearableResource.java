package it.unito.sensors.server.controller;

import it.unito.sensors.server.entity.WearableEntity;
import it.unito.sensors.server.repository.WearableRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * \brief REST API to handle wearables
 */
@RestController
public class WearableResource {

	@Autowired
    private WearableRepository wearableRepository;

    @GetMapping(
            value = "/wearable"
    )
    public List<WearableEntity> retrieveAllNodes() {
        return wearableRepository.findAll();
    }

    @GetMapping(
            value = "/wearable/{id}",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public WearableEntity retrieveWearable(@PathVariable long id) {
        return wearableRepository.findById(id);
    }

    @PostMapping(
            value = "/wearable",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public ResponseEntity<WearableEntity> createWearable(@RequestParam String code) {
        WearableEntity wearableEntityToFind = wearableRepository.findByCode(code);

        if (wearableEntityToFind != null) {
            return ResponseEntity.status(HttpStatus.BAD_REQUEST).build();
        }

        WearableEntity wearableEntity = new WearableEntity(code);
        wearableRepository.save(wearableEntity);
        return ResponseEntity.ok(wearableEntity);
    }
}