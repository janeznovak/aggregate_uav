package it.unito.sensors.server.fs;

import it.unito.sensors.server.fs.dto.CSVWearableSosDTO;
import it.unito.sensors.server.fs.util.CSVReaderUtil;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;

/**
 * \brief WearableSosReader transforms a csv file containing wearable sos to DTOs
 */
@Service
@Slf4j
public class WearableSosReader implements Reader<CSVWearableSosDTO> {

    @Override
    public List<CSVWearableSosDTO> read(String filePath) {
        try {
            CSVReaderUtil csvReaderUtil = new CSVReaderUtil();
            List<CSVWearableSosDTO> csvWearableSosDTOS = csvReaderUtil.readBeansFromFile(CSVWearableSosDTO.class, filePath);

            return csvWearableSosDTOS;
        } catch (Exception e) {
            log.error("Error reading file", e);
            return new ArrayList<>();
        }
    }

}
