package it.unito.sensors.server.entity;

import java.util.Arrays;

/**
 * \brief Enum represents list of possible goals of the goal state machine (see docs to view the state diagram).
 *
 * External code is used by other components of the system (Navigation_System, AP etc...)
 */
public enum GoalStatusEnum implements EnumCode {

    NO_GOAL("NO_GOAL", -1),
    REACHED("REACHED",0),
    ABORTED("ABORTED", 1),
    FAILED("FAILED", 2),
    RUNNING("RUNNING", 3),
    UNKNOWN("UNKNOWN", 4);

    GoalStatusEnum(String code, Integer externalCode) {
        this.code = code;
        this.externalCode = externalCode;
    }

    public String getCode() {
        return code;
    }

    final private String code;
    final private Integer externalCode;

    public static GoalStatusEnum getEnumByExternalCode(Integer intCode) {
        return Arrays.stream(GoalStatusEnum.values())
                .filter(e -> e.externalCode.equals(intCode))
                .findAny()
                .orElse(null);
    }

}
