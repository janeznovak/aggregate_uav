package it.unito.sensors.server.configuration.dto;

import lombok.Getter;
import lombok.Setter;

import java.util.Map;

@Getter
@Setter
public class GoalStateMachineProperties {

    Map<String, CallbackConfigProperties> callback;
    Map<String, Integer> timeout;
}
