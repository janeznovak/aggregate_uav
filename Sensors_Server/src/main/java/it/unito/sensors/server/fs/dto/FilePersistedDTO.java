package it.unito.sensors.server.fs.dto;

import lombok.Builder;
import lombok.Data;

@Data
@Builder
public class FilePersistedDTO {
    String path;
    String absolutePath;
    byte [] data;
}
