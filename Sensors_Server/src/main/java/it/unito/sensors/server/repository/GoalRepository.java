package it.unito.sensors.server.repository;

import it.unito.sensors.server.dto.NodeReportDTO;
import it.unito.sensors.server.dto.StatusReportDTO;
import it.unito.sensors.server.entity.GoalEntity;
import it.unito.sensors.server.entity.GoalStatusEnum;
import it.unito.sensors.server.entity.NodeEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.stereotype.Repository;

import java.util.List;

/**
 * \brief The repository of Goal Entity.
 */
@Repository
public interface GoalRepository extends JpaRepository<GoalEntity, Long>{

    GoalEntity findByCode(String code);

    List<GoalEntity> findByNodeEntity(NodeEntity nodeEntity);
    List<GoalEntity> findByNodeEntityAndActionIn(NodeEntity nodeEntity, String[] action);

    GoalEntity findByStatusNotAndReferenceCodeAndAction(GoalStatusEnum goalStatusEnum, String referenceCode, String action);

    GoalEntity findTop1ByStatusAndReferenceCodeAndActionOrderByTimestampUpdateDesc(GoalStatusEnum goalStatusEnum, String referenceCode, String action);

    GoalEntity findTop1ByNodeEntityOrderByTimestampCreationDesc(NodeEntity nodeEntity);

    GoalEntity findTop1ByNodeEntityAndStatusNotOrderByTimestampUpdateDesc(NodeEntity nodeEntity, GoalStatusEnum goalStatusEnumExcluded);

    List<GoalEntity> findByActionInOrderByTimestampCreationDesc(String[] action);

    List<GoalEntity> findByOrderByTimestampCreationDesc();

    @Query("SELECT new it.unito.sensors.server.dto.NodeReportDTO(g.nodeEntity.id, count(*)) " +
            "FROM GOAL g " +
            "WHERE g.nodeEntity is not null " +
            "GROUP BY g.nodeEntity.id")
    List<NodeReportDTO> goalsByNode();

    @Query("SELECT new it.unito.sensors.server.dto.StatusReportDTO(g.status, count(*)) " +
            "FROM GOAL g " +
            "WHERE g.status is not null " +
            "GROUP BY g.status")
    List<StatusReportDTO> goalsByStatus();
}