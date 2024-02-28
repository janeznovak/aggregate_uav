package it.unito.sensors.server.controller;

import it.unito.sensors.server.dto.MeasurementDTO;
import it.unito.sensors.server.dto.mapper.GoalDTOMapper;
import it.unito.sensors.server.dto.mapper.MeasurementDTOMapper;
import it.unito.sensors.server.entity.*;
import it.unito.sensors.server.repository.DeviceRepository;
import it.unito.sensors.server.repository.MeasurementRepository;
import it.unito.sensors.server.repository.NeighbourRepository;
import it.unito.sensors.server.repository.NodeRepository;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

import it.unito.sensors.server.service.MeasurementService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.domain.Example;
import org.springframework.data.domain.Pageable;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

/**
 * \brief REST API to handle measurements from sensors equipped on boards
 */
@RestController
@Slf4j
public class MeasurementResource {
	@Autowired
    private MeasurementRepository measurementRepository;

    @Autowired
    MeasurementService measurementService;

    @Autowired
    MeasurementDTOMapper measurementDTOMapper;

    @GetMapping(
            value = "/measurements",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public List<MeasurementDTO> retrieveAllMeasurements() {
        return measurementRepository
                .findAll()
                .stream()
                .map(measurementDTOMapper::map)
                .collect(Collectors.toList());
    }

    @GetMapping(
            value = "/measurements/last",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public MeasurementDTO retrieveLastMeasurement() {
        return Optional.ofNullable(measurementRepository.findTop1ByOrderByUnixTimestampDesc())
                .map(measurementDTOMapper::map)
                .orElse(null);
    }

    @GetMapping(
            value = "/measurements/node/{nodeId}/last",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public MeasurementDTO retrieveLastMeasurement(@PathVariable long nodeId) {
        return Optional.ofNullable(
                    measurementRepository.findTop1ByNodeEntityOrderByUnixTimestampDesc(
                            NodeEntity.builder().id(nodeId).build()
                    )
                )
                .map(measurementDTOMapper::map)
                .orElse(null);
    }

    @GetMapping(
            value = "/measurements/node/{nodeId}",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public List<MeasurementDTO> retrieveMeasurementsByNode(@PathVariable long nodeId) {
        NodeEntity nodeEntity = NodeEntity.builder().id(nodeId).build();
        Example<MeasurementEntity> example = Example.of(MeasurementEntity.builder().nodeEntity(nodeEntity).build());
        return measurementRepository
                .findAll(example, Pageable.unpaged())
                .stream()
                .map(measurementDTOMapper::map)
                .collect(Collectors.toList());
    }


    @PostMapping(
            value = "/measurements",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public ResponseEntity<MeasurementDTO> createMeasurement(
            @RequestParam long nodeId,
            @RequestBody MeasurementDTO measurementDTO) {

        boolean status = measurementService.createMeasurement(nodeId, measurementDTO);
        if (status) {
            return ResponseEntity.ok().build();
        } else {
            return ResponseEntity
                    .status(HttpStatus.BAD_REQUEST)
                    .build();
        }
    }

}