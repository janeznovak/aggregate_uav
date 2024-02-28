package it.unito.sensors.server.util;

import it.unito.sensors.server.configuration.AppConfiguration;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Path;
import java.util.concurrent.CompletableFuture;
import java.util.stream.Collectors;

@Component
@Slf4j
/**
 * \brief Utility to run bash scripts saved in APP_SCRIPTS_PATH
 */
public class BashScriptRunner {

    @Autowired
    AppConfiguration appConfiguration;

    /**
     * Runs a bash script in a syncronous way.
     * @param scriptName script name
     * @param args optional arguments to pass
     * @return std out and std err
     * @throws IOException
     * @throws InterruptedException
     */
    public String runBashScript(String scriptName, String... args) throws IOException, InterruptedException {
        String pathFolder = appConfiguration.getScripts().getPath();
        Path directory = Path.of(pathFolder);

        String script = getScript(scriptName, directory, args);
        log.debug("Run script: {}", script);

        Process p = Runtime.getRuntime().exec(new String[]{"sh", "-c", script});

        int exitCode = p.waitFor();
        log.info("exit code: {}", exitCode);

        BufferedReader lineReader = new BufferedReader(new InputStreamReader(p.getInputStream()));
        lineReader.lines().forEach(log::info);

        BufferedReader errorReader = new BufferedReader(new InputStreamReader(p.getErrorStream()));
        errorReader.lines().forEach(log::error);

        return lineReader.lines().collect(Collectors.joining(System.lineSeparator()));
    }

    /**
     * Runs a bash script in async way.
     * @param scriptName script name
     * @param args optional arguments to pass
     * @return completable future to used
     */
    public CompletableFuture<String> runBashScriptAsync(String scriptName, String... args) {
        return CompletableFuture.supplyAsync(() -> {
            try {
                String pathFolder = appConfiguration.getScripts().getPath();
                Path directory = Path.of(pathFolder);

                String script = getScript(scriptName, directory, args);
                log.debug("Run script: {}", script);

                ProcessBuilder processBuilder = new ProcessBuilder("sh", "-c", script);
                processBuilder.redirectErrorStream(true);
                Process p = processBuilder.start();

                try (BufferedReader reader = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
                    return reader.lines().collect(Collectors.joining(System.lineSeparator()));
                }
            } catch (IOException e) {
                log.error("Error running script", e);
                return "Error running script";
            }
        });
    }

    /**
     * Returns the command to run.
     * @param scriptName
     * @param directory
     * @param args
     * @return
     */
    private String getScript(String scriptName, Path directory, String[] args) {
        String script = directory + "/" + scriptName + " " + String.join(" ", args);
        return script;
    }
}
