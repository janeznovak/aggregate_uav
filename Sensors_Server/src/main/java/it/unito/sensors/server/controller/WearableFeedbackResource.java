package it.unito.sensors.server.controller;

import it.unito.sensors.server.dto.WearableFeedbackDTO;
import it.unito.sensors.server.dto.mapper.WearableFeedbackDTOMapper;
import it.unito.sensors.server.entity.WearableEntity;
import it.unito.sensors.server.entity.WearableFeedbackEntity;
import it.unito.sensors.server.repository.WearableFeedbackRepository;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.domain.Example;
import org.springframework.data.domain.Pageable;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * \brief REST API to handle feedbacks from wearables
 */
@RestController
@Slf4j
public class WearableFeedbackResource {

	@Autowired
    private WearableFeedbackRepository wearableFeedbackRepository;

    @Autowired
    private WearableFeedbackDTOMapper wearableFeedbackDTOMapper;

    @GetMapping(
            value = "/wearable-feedbacks",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public List<WearableFeedbackDTO> retrieveAllFeedbacks() {
        return wearableFeedbackRepository
                .findAll()
                .stream()
                .map(wearableFeedbackDTOMapper::map)
                .collect(Collectors.toList());
    }

    @GetMapping(
            value = "/wearable-feedbacks/last",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public WearableFeedbackDTO retrieveLastFeedback() {
        return Optional.ofNullable(wearableFeedbackRepository.findTop1ByOrderByUnixTimestampDesc())
                .map(wearableFeedbackDTOMapper::map)
                .orElse(null);
    }

    @GetMapping(
            value = "/wearable-feedbacks/wearable/{wearableId}/last",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public WearableFeedbackDTO retrieveLastFeedbackByWearable(@PathVariable long wearableId) {
        return Optional.ofNullable(
                        wearableFeedbackRepository.findTop1ByWearableEntityOrderByUnixTimestampDesc(
                            WearableEntity.builder().id(wearableId).build()
                    )
                )
                .map(wearableFeedbackDTOMapper::map)
                .orElse(null);
    }

    @GetMapping(
            value = "/wearable-feedbacks/wearable/{wearableId}",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public List<WearableFeedbackDTO> retrieveFeedbacksByWearable(@PathVariable long wearableId) {
        WearableEntity wearableEntity = WearableEntity.builder().id(wearableId).build();
        Example<WearableFeedbackEntity> example = Example.of(WearableFeedbackEntity.builder().wearableEntity(wearableEntity).build());
        return wearableFeedbackRepository
                .findAll(example, Pageable.unpaged())
                .stream()
                .map(wearableFeedbackDTOMapper::map)
                .collect(Collectors.toList());
    }

}