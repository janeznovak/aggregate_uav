package it.unito.sensors.server.entity;

import com.fasterxml.jackson.annotation.JsonIgnore;
import lombok.*;

import javax.persistence.*;
import java.time.LocalDateTime;
import java.util.List;

/**
 * \brief This entity represents a set of data coming from a node at a specific time
 */
@Entity
@AllArgsConstructor
@NoArgsConstructor
@Builder
@Getter
@Setter
@Table(name = "MEASUREMENTS",
       indexes = {@Index(columnList = "UNIXTIMESTAMP")},
       uniqueConstraints={@UniqueConstraint(name="UN_MEASUREMENT_ID", columnNames={"ID"})}
)
public class MeasurementEntity {

    /**
     * Auto-generated identifier to have a unique key for this sensor.
     */
    @Id
    @GeneratedValue
    private Long id;

    /**
     * Relationship between the measurement and its sensor.
     */
    @ManyToOne
    @JoinColumn(name = "NODE_ID", nullable = false, foreignKey = @ForeignKey(name="FK_MEASUREMENT_NODE"))
    @JsonIgnore
    private NodeEntity nodeEntity;

    /**
     * Timestamp (unix timestamp, milliseconds) of the measurement.
     */
    @Column(nullable = false)
    private Long unixTimestamp;

    /**
     * Timestamp (timestamp) of the measurement.
     */
    @Column(nullable = false)
    private LocalDateTime timestamp;

    /**
     * List of neighbours
     */
    @OneToMany(fetch = FetchType.EAGER)
    @JoinColumn(name = "MEASUREMENT_ID")
    private List<NeighbourEntity> neighbours;

    /**
     * Data about this measure
     */
    @Embedded
    MeasurementData measurementData;
}
