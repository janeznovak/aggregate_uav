package it.unito.sensors.server.configuration.dto;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

@Getter
@Setter
public class CallbackConfigProperties {

    public static final String BASH_TYPE = "bash";

    String type;
    String name;
    List<String> goalActions;

}
