package it.unito.sensors.server.dto.mapper;

import it.unito.sensors.server.dto.DTO;

/**
 * \brief Interface used to marshal/unmarshal operations of DTOs
 */
public interface DTOMapper<IN, OUT extends DTO> {

    OUT map(IN dto);
}
