package it.unito.sensors.server.controller;

import it.unito.sensors.server.dto.FeedbackDTO;
import it.unito.sensors.server.dto.mapper.FeedbackDTOMapper;
import it.unito.sensors.server.entity.FeedbackEntity;
import it.unito.sensors.server.entity.NodeEntity;
import it.unito.sensors.server.repository.FeedbackRepository;
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
 * \brief REST API to handle feedbacks from robots
 */
@RestController
@Slf4j
public class FeedbackResource {

	@Autowired
    private FeedbackRepository feedbackRepository;

    @Autowired
    private FeedbackDTOMapper feedbackDTOMapper;

    @GetMapping(
            value = "/feedbacks",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public List<FeedbackDTO> retrieveAllFeedbacks() {
        return feedbackRepository
                .findAll()
                .stream()
                .map(feedbackDTOMapper::map)
                .collect(Collectors.toList());
    }

    @GetMapping(
            value = "/feedbacks/last",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public FeedbackDTO retrieveLastFeedback() {
        return Optional.ofNullable(feedbackRepository.findTop1ByOrderByUnixTimestampDesc())
                .map(feedbackDTOMapper::map)
                .orElse(null);
    }

    @GetMapping(
            value = "/feedbacks/node/{nodeId}/last",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public FeedbackDTO retrieveLastFeedbackByNode(@PathVariable long nodeId) {
        return Optional.ofNullable(
                    feedbackRepository.findTop1ByNodeEntityOrderByUnixTimestampDesc(
                            NodeEntity.builder().id(nodeId).build()
                    )
                )
                .map(feedbackDTOMapper::map)
                .orElse(null);
    }

    @GetMapping(
            value = "/feedbacks/node/{nodeId}",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public List<FeedbackDTO> retrieveFeedbacksByNode(@PathVariable long nodeId) {
        NodeEntity nodeEntity = NodeEntity.builder().id(nodeId).build();
        Example<FeedbackEntity> example = Example.of(FeedbackEntity.builder().nodeEntity(nodeEntity).build());
        return feedbackRepository
                .findAll(example, Pageable.unpaged())
                .stream()
                .map(feedbackDTOMapper::map)
                .collect(Collectors.toList());
    }

}