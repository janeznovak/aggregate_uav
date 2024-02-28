package it.unito.sensors.server.fs.util;

import it.unito.sensors.server.configuration.AppConfiguration;

public class FsUtil {

    public static String getRobotPathFolder(String path, AppConfiguration appConfiguration, String robotName) {
        return path.replaceAll(appConfiguration.getPlaceholder().getRobot(), robotName);
    }

    public static String getWearablePathFolder(String path, AppConfiguration appConfiguration, String wearableCode) {
        return path.replaceAll(appConfiguration.getPlaceholder().getWearable(), wearableCode);
    }
}
