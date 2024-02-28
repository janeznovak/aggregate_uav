package it.unito.sensors.server.configuration.dto;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

@Getter
@Setter
public class RobotProperties {

    List<String> names;
    String example;
    Integer count;

}
