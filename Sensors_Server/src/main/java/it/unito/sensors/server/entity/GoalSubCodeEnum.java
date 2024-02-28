package it.unito.sensors.server.entity;

import java.util.Arrays;

/**
 * \brief Enum represents list of possible sub code of the goal.
 *
 */
public enum GoalSubCodeEnum implements EnumCode {

    NONE("NONE"),
    FOLLOW("FOLLOW"),
    ACCIDENTAL_FALL("ACCIDENTAL_FALL"),
    HEART("HEART"),
    OXYGENATION("OXYGENATION"),
    BODY_TEMPERATURE("BODY_TEMPERATURE");

    GoalSubCodeEnum(String code) {
        this.code = code;
    }

    public String getCode() {
        return code;
    }

    final private String code;

    public static GoalSubCodeEnum getEnumByCode(String code) {
        return Arrays.stream(GoalSubCodeEnum.values())
                .filter(e -> e.code.equals(code))
                .findAny()
                .orElse(null);
    }

}
