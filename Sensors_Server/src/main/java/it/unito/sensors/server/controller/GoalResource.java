package it.unito.sensors.server.controller;

import it.unito.sensors.server.dto.GoalDTO;
import it.unito.sensors.server.dto.mapper.GoalDTOMapper;
import it.unito.sensors.server.entity.GoalEntity;
import it.unito.sensors.server.entity.NodeEntity;
import it.unito.sensors.server.repository.GoalRepository;
import it.unito.sensors.server.service.GoalService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.stream.Collectors;

/**
 * \brief REST API to handle goals of robots (GOAL, ABORT, DOCK...)
 */
@RestController
@Slf4j
public class GoalResource {

    @Autowired
    GoalService goalService;

    @Autowired
    private GoalDTOMapper goalDTOMapper;

    @Autowired
    GoalRepository goalRepository;

    @GetMapping(
            value = "/goals",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public List<GoalDTO> retrieveAllGoals(@RequestParam(required = false) String[] actions) {
        List<GoalEntity> goalEntities;
        if (actions == null || actions.length == 0) {
            goalEntities = goalRepository.findByOrderByTimestampCreationDesc();
        } else {
            goalEntities = goalRepository.findByActionInOrderByTimestampCreationDesc(actions);
        }

        return goalEntities
                .stream()
                .map(goalDTOMapper::map)
                .collect(Collectors.toList());
    }

    @GetMapping(
            value = "/goals/node/{nodeId}",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public List<GoalDTO> retrieveGoalsByNodeAndActions(@PathVariable long nodeId, @RequestParam(required = false) String[] actions) {
        List<GoalEntity> goalEntities;
        NodeEntity nodeEntity = NodeEntity.builder().id(nodeId).build();
        if (actions == null || actions.length == 0) {
            goalEntities = goalRepository.findByNodeEntity(nodeEntity);
        } else {
            goalEntities = goalRepository.findByNodeEntityAndActionIn(nodeEntity, actions);
        }
        return goalEntities
                .stream()
                .map(goalDTOMapper::map)
                .collect(Collectors.toList());
    }

    @PostMapping(
            value = "/goals",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public ResponseEntity<GoalDTO> createGoal(
            @RequestBody GoalDTO goalDTO) {

        boolean status = goalService.createGoal(goalDTO);
        if (status) {
            return ResponseEntity.ok(goalDTO);
        } else {
            return ResponseEntity
                    .status(HttpStatus.BAD_REQUEST)
                    .build();
        }
    }

}