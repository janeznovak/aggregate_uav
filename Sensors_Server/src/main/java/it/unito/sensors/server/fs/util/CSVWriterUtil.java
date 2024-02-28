package it.unito.sensors.server.fs.util;

import com.opencsv.CSVWriter;
import org.springframework.stereotype.Service;

import java.io.*;
import java.util.List;

/**
 * \brief Convert a csv file to a list of pojo, with some configuration in this class and in pojo class.
 */
@Service
public class CSVWriterUtil {
    private static char SEPARATOR = ';';

    public byte[] writeToFile(String filePath, List<String[]> dataLines) throws IOException {
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        OutputStreamWriter streamWriter = new OutputStreamWriter(stream);
        CSVWriter writer = new CSVWriter(streamWriter,
                                        SEPARATOR,
                                        CSVWriter.NO_QUOTE_CHARACTER,
                                        CSVWriter.DEFAULT_ESCAPE_CHARACTER,
                                        CSVWriter.DEFAULT_LINE_END
                );

        writer.writeAll(dataLines);
        streamWriter.flush();
        byte[] bytes = stream.toByteArray();

        File outputFile = new File(filePath.replaceAll("\\.txt$", ".lock"));
        try (FileOutputStream outputStream = new FileOutputStream(outputFile)) {
            outputStream.write(bytes);
        }
        outputFile.renameTo(new File(filePath));

        // closing writer connection
        writer.close();

        return bytes;
    }

}
