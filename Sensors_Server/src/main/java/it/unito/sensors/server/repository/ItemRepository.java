package it.unito.sensors.server.repository;

import it.unito.sensors.server.entity.ItemEntity;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

/**
 * \brief The repository of Item Entity.
 */
@Repository
public interface ItemRepository extends JpaRepository<ItemEntity, Long>{

    ItemEntity findByCode(String code);
}