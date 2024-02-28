package it.unito.sensors.server.service;

import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Component;

import java.io.FileWriter;
import java.io.IOException;
import java.nio.charset.Charset;
import java.nio.file.Path;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * \brief StorageService exposes some utils to better "use" the Storage folder.
 */
@Component
@Slf4j
public class StorageService {

    static final String STORAGE_FILE_NAME = "Storage";
    String regexStorage = STORAGE_FILE_NAME+"(/.*)";
    Pattern pattern = Pattern.compile(regexStorage);

    /**
     * Copy binary data to a file in the Storage
     * @param data
     * @param charset
     * @param fileName
     */
    public void copyFileToStorageFolder(byte[] data, Charset charset, String fileName) {
        try {
            FileWriter myWriter = new FileWriter(fileName);
            myWriter.write(new String(data, charset));
            myWriter.close();
        } catch (IOException e) {
            log.error("Error copy file to storage folder", e);
        }
    }

    /**
     * Convert an absolute path to relative path, using the Storage path as base path.
     * Keeps "/Storage" token of absolute path if keepStorageFolder is true.
     *
     * @param absolutePath
     * @param keepStorageFolder
     * @return
     */
    public String convertToStoragePath(String absolutePath, Boolean keepStorageFolder) {
        Matcher matcher = pattern.matcher(absolutePath);
        String pathWithStorageAsBasePath = absolutePath;
        if (matcher.find()) {
            pathWithStorageAsBasePath = matcher.group(1);
        }
        if (keepStorageFolder) {
           return STORAGE_FILE_NAME + pathWithStorageAsBasePath;
        } else {
            return pathWithStorageAsBasePath;
        }
    }

}
