package it.unito.sensors.server.repository;

import it.unito.sensors.server.entity.WearableEntity;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

/**
 * \brief The repository of Wearable Entity.
 */
@Repository
public interface WearableRepository extends JpaRepository<WearableEntity, Long>{

    Page<WearableEntity> findAll(Pageable pageable);

    WearableEntity findByCode(String code);

    WearableEntity findById(@Param("id") long id);
}