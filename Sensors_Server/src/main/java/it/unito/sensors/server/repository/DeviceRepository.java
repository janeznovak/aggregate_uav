package it.unito.sensors.server.repository;

import it.unito.sensors.server.entity.DeviceEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

/**
 * \brief The repository of Device Entity.
 */
@Repository
public interface DeviceRepository extends JpaRepository<DeviceEntity, Long>{

    DeviceEntity findByMacAddress(String macAddress);
}