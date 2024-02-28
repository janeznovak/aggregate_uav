package it.unito.sensors.server.repository;

import it.unito.sensors.server.entity.FeedbackEntity;
import it.unito.sensors.server.entity.NodeEntity;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

/**
 * \brief The repository of Feedback Entity.
 */
@Repository
public interface FeedbackRepository extends JpaRepository<FeedbackEntity, Long>{

    Page<FeedbackEntity> findAll(Pageable pageable);

    FeedbackEntity findTop1ByOrderByUnixTimestampDesc();

    FeedbackEntity findTop1ByNodeEntityOrderByUnixTimestampDesc(NodeEntity nodeEntity);
}