package it.unito.sensors.server.fs;

import java.util.List;

/**
 * \brief A generic interface to unmarshal some data from files.
 */
public interface Reader<DTO> {

    public List<DTO> read(String filePath);

}
