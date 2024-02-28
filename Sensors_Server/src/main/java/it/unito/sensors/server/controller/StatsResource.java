package it.unito.sensors.server.controller;

import it.unito.sensors.server.dto.NodeReportDTO;
import it.unito.sensors.server.dto.StatusReportDTO;
import it.unito.sensors.server.dto.mapper.GoalDTOMapper;
import it.unito.sensors.server.repository.GoalRepository;
import it.unito.sensors.server.service.GoalService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.*;

import java.util.List;

/**
 * \brief REST API to extract statistics
 */
@RestController
@Slf4j
public class StatsResource {

    @Autowired
    GoalService goalService;

    @Autowired
    private GoalDTOMapper goalDTOMapper;

    @Autowired
    GoalRepository goalRepository;

    @GetMapping(
            value = "/stats/total-goals-by-node",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public List<NodeReportDTO> totalGoalsByRobot() {
        return goalRepository
                .goalsByNode();
    }

    @GetMapping(
            value = "/stats/total-goals-by-status",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public List<StatusReportDTO> totalGoalsByStatus() {
        return goalRepository
                .goalsByStatus();
    }
}