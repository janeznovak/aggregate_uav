package it.unito.sensors.server.service.util;

import lombok.AllArgsConstructor;
import lombok.Getter;

import java.nio.file.WatchKey;

@Getter
@AllArgsConstructor
class PathWatcher {
    protected String path;
    protected WatchKey watchKey;
}