package it.unito.sensors.server.configuration.dto;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

@Getter
@Setter
public class WearableProperties {

    List<String> codes;
    String example;
    Integer count;

}
