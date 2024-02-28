package it.unito.sensors.server.fs;

import it.unito.sensors.server.fs.dto.FilePersistedDTO;
import java.util.Optional;

/**
 * \brief A generic interface to marshal objects to files.
 */
public interface Writer<DTO> {

    public Optional<FilePersistedDTO> write(DTO dto);

}
