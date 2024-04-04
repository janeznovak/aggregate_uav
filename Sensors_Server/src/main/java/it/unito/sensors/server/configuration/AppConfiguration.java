package it.unito.sensors.server.configuration;

import it.unito.sensors.server.configuration.dto.*;
import lombok.Getter;
import lombok.Setter;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.context.annotation.Configuration;

import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

/**
 * \brief Configuration of APP regarding storage, etc.
 *
 */
@Getter
@Setter
@Configuration
@ConfigurationProperties(prefix = "app")
public class AppConfiguration {

    GoalProperties goal;
    ActionProperties action;
    StorageProperties storage;
    ScriptsProperties scripts;
    PlaceholderProperties placeholder;
    RobotProperties robot;
    FeedbackProperties feedback;
    GoalStateMachineProperties goalStateMachine;

    public List<String> getRobotNames() {
        if (robot.getNames() != null  && !robot.getNames().isEmpty()) {
            return robot.getNames();
        } else {
            return IntStream.range(1, robot.getCount())
                    .mapToObj(i -> robot.getExample().replaceAll(placeholder.getRobot(), Integer.toString(i)))
                    .collect(Collectors.toList());
        }
    }
}
