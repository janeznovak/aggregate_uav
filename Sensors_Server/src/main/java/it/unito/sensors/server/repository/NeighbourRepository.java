package it.unito.sensors.server.repository;

import it.unito.sensors.server.entity.NeighbourEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

/**
 * \brief The repository of Neighbour Entity.
 */
@Repository
public interface NeighbourRepository extends JpaRepository<NeighbourEntity, Long>{

}