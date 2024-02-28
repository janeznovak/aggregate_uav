package it.unito.sensors.server.repository;

import it.unito.sensors.server.entity.WearableEntity;
import it.unito.sensors.server.entity.WearableFeedbackEntity;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

/**
 * \brief The repository of Feedback Entity.
 */
@Repository
public interface WearableFeedbackRepository extends JpaRepository<WearableFeedbackEntity, Long>{

    Page<WearableFeedbackEntity> findAll(Pageable pageable);

    WearableFeedbackEntity findTop1ByOrderByUnixTimestampDesc();

    WearableFeedbackEntity findTop1ByWearableEntityOrderByUnixTimestampDesc(WearableEntity wearableEntity);
}