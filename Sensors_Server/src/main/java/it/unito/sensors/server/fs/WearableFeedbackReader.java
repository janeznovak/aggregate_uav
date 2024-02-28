package it.unito.sensors.server.fs;

import it.unito.sensors.server.fs.dto.CSVWearableFeedbackDTO;
import it.unito.sensors.server.fs.util.CSVReaderUtil;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;

/**
 * \brief WearableFeedbackReader transforms a csv file containing wearable feedbacks to DTOs
 */
@Service
@Slf4j
public class WearableFeedbackReader implements Reader<CSVWearableFeedbackDTO> {

    @Override
    public List<CSVWearableFeedbackDTO> read(String filePath) {
        try {
            CSVReaderUtil csvReaderUtil = new CSVReaderUtil();
            List<CSVWearableFeedbackDTO> csvWearableFeedbackDTOS = csvReaderUtil.readBeansFromFile(CSVWearableFeedbackDTO.class, filePath);

            return csvWearableFeedbackDTOS;
        } catch (Exception e) {
            log.error("Error reading file", e);
            return new ArrayList<>();
        }
    }

}
