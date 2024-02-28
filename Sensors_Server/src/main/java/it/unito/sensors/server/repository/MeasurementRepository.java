package it.unito.sensors.server.repository;

import it.unito.sensors.server.entity.MeasurementEntity;
import it.unito.sensors.server.entity.NodeEntity;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

/**
 * \brief The repository of Measurement Entity.
 */
@Repository
public interface MeasurementRepository extends JpaRepository<MeasurementEntity, Long>{

    Page<MeasurementEntity> findAll(Pageable pageable);

    MeasurementEntity findTop1ByOrderByUnixTimestampDesc();

    MeasurementEntity findTop1ByNodeEntityOrderByUnixTimestampDesc(NodeEntity nodeEntity);
}