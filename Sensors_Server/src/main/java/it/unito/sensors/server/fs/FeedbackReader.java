package it.unito.sensors.server.fs;

import it.unito.sensors.server.fs.dto.CSVFeedbackDTO;
import it.unito.sensors.server.fs.util.CSVReaderUtil;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;

/**
 * \brief FeedbackReader transforms a csv file containing feedbacks to DTOs
 */
@Service
@Slf4j
public class FeedbackReader implements Reader<CSVFeedbackDTO> {

    @Override
    public List<CSVFeedbackDTO> read(String filePath) {
        try {
            CSVReaderUtil csvReaderUtil = new CSVReaderUtil();
            List<CSVFeedbackDTO> csvFeedbackDTOS = csvReaderUtil.readBeansFromFile(CSVFeedbackDTO.class, filePath);

            return csvFeedbackDTOS;
        } catch (Exception e) {
            log.error("Error reading file", e);
            return new ArrayList<>();
        }
    }

}
