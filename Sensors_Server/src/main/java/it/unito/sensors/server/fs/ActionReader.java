package it.unito.sensors.server.fs;

import it.unito.sensors.server.fs.dto.CSVActionDTO;
import it.unito.sensors.server.fs.util.CSVReaderUtil;
import lombok.extern.slf4j.Slf4j;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;

/**
 * \brief ActionReader transforms a csv file containing actions to DTOs
 */
@Service
@Slf4j
public class ActionReader implements Reader<CSVActionDTO> {

    @Override
    public List<CSVActionDTO> read(String filePath) {
        try {
            CSVReaderUtil csvReaderUtil = new CSVReaderUtil();
            List<CSVActionDTO> csvActionDTOS = csvReaderUtil.readBeansFromFile(CSVActionDTO.class, filePath);

            return csvActionDTOS;
        } catch (Exception e) {
            log.error("Error reading file", e);
            return new ArrayList<>();
        }
    }

}
