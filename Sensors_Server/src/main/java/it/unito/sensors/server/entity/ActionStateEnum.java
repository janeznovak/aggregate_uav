package it.unito.sensors.server.entity;

import lombok.Getter;

/**
 * \brief Enum represents list of possible actions
 */
@Getter
public enum ActionStateEnum {

    GOAL("GOAL"),
    ABORT("ABORT"),
    SOS("SOS"),
    FOLLOW("FOLLOW");

    ActionStateEnum(String code) {
        this.code = code;
    }

    private String code;
}
