package it.unito.sensors.server.repository;

import it.unito.sensors.server.entity.NodeEntity;
import java.util.List;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

/**
 * \brief The repository of Node Entity.
 */
@Repository
public interface NodeRepository extends JpaRepository<NodeEntity, Long>{

    Page<NodeEntity> findAll(Pageable pageable);

    NodeEntity findByName(String name);

    NodeEntity findById(@Param("id") long id);
}