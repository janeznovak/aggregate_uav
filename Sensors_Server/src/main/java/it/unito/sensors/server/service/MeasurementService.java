package it.unito.sensors.server.service;

import it.unito.sensors.server.dto.MeasurementDTO;
import it.unito.sensors.server.entity.DeviceEntity;
import it.unito.sensors.server.entity.MeasurementEntity;
import it.unito.sensors.server.entity.NeighbourEntity;
import it.unito.sensors.server.entity.NodeEntity;
import it.unito.sensors.server.repository.DeviceRepository;
import it.unito.sensors.server.repository.MeasurementRepository;
import it.unito.sensors.server.repository.NeighbourRepository;
import it.unito.sensors.server.repository.NodeRepository;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import javax.transaction.Transactional;
import java.time.LocalDateTime;

/**
 * \brief MeasurementService exposes to other services some extended features about measurements.
 */
@Slf4j
@Service
public class MeasurementService {

    @Autowired
    private NodeRepository nodeRepository;

    @Autowired
    private MeasurementRepository measurementRepository;

    @Autowired
    private DeviceRepository deviceRepository;

    @Autowired
    private NeighbourRepository neighbourRepository;

    /**
     * Create new measurement coming from a node in a database transaction
     * @param nodeId
     * @param measurementDTO
     * @return
     */
    @Transactional
    public boolean createMeasurement(long nodeId,
                                    MeasurementDTO measurementDTO) {
        NodeEntity nodeEntity = nodeRepository.findById(nodeId);

        // create node entity if not exists
        if (nodeEntity == null) {
            log.error("nodeId {} not exists", nodeId);
            return false;
        }

        LocalDateTime now = LocalDateTime.now();

        // create measurement with data
        MeasurementEntity measurementEntity = MeasurementEntity.builder()
                .nodeEntity(nodeEntity)
                .timestamp(now)
                .unixTimestamp(System.currentTimeMillis())
                .measurementData(measurementDTO.getMeasurementData())
                .build();
        measurementRepository.save(measurementEntity);
        log.debug("measurement saved");

        if (measurementDTO.getNeighbours() != null) {
            log.debug("found {} neighbours data to save", measurementDTO.getNeighbours().size());
            for (NeighbourEntity neighbourEntity : measurementDTO.getNeighbours()){
                DeviceEntity device = neighbourEntity.getDevice();

                DeviceEntity deviceFound = deviceRepository.findByMacAddress(device.getMacAddress());

                if (deviceFound == null) {
                    log.debug("create new device");
                    // create device if not exists
                    DeviceEntity deviceEntitySaved = deviceRepository.save(device);
                    device.setId(deviceEntitySaved.getId());
                } else {
                    device.setId(deviceFound.getId());
                    // update device if is changed
                    if (!deviceFound.getLocalName().equals(device.getLocalName())) {
                        log.debug("update device");
                        deviceRepository.save(device);
                    } else {
                        // NOOP
                        log.debug("no update device");
                    }
                }

                // link measurement neighbour to previous measurement created
                neighbourEntity.setMeasurementEntity(measurementEntity);
            }
            // save all measurement neighbours
            neighbourRepository.saveAll(measurementDTO.getNeighbours());
            log.debug("neighbours saved");
        }

        return true;
    }

}
