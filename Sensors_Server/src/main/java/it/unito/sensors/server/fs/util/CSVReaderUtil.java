package it.unito.sensors.server.fs.util;

import com.opencsv.CSVWriter;
import com.opencsv.bean.CsvToBeanBuilder;

import java.io.FileReader;
import java.io.IOException;
import java.util.List;

/**
 * \brief Convert a csv file to a list of pojo, with some configuration in this class and in pojo class.
 */
public class CSVReaderUtil {

    private static char SEPARATOR = ';';

    public <T> List<T> readBeansFromFile(Class<T> clazz, String filePath) throws IOException {
        try (FileReader fileReader = new FileReader(filePath)) {
            List<T> actions = new CsvToBeanBuilder<T>(fileReader)
                    .withSeparator(SEPARATOR)
                    .withQuoteChar(CSVWriter.NO_QUOTE_CHARACTER)
                    .withEscapeChar(CSVWriter.DEFAULT_ESCAPE_CHARACTER)
                    .withType(clazz)
                    .build()
                    .parse();
            return actions;
        }
    }

}
