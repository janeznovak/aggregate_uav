package it.unito.sensors.server.entity;

import lombok.*;
import net.minidev.json.annotate.JsonIgnore;

import java.util.HashSet;
import java.util.Set;
import javax.persistence.*;
/**
 * \brief This entity represents a physical object (like robots, drones etc...) that "live" in a world.
 *
 * They can move in a room, patrol a room or find an object in it. They can be equipped with some sensors, like pressure and temperature sensor.
 */
@Entity
@AllArgsConstructor
@NoArgsConstructor
@Builder
@Getter
@Setter
@EqualsAndHashCode
@Table(name = "NODES",
        indexes = @Index(columnList = "NAME"),
        uniqueConstraints={@UniqueConstraint(name="UN_NODE_NAME", columnNames={"NAME"})})
public class NodeEntity {

    /**
     * Auto-generated identifier to have a unique key for this sensor.
     */
    @Id
    @GeneratedValue(
            generator = "SEQ_NODE"
    )
    @SequenceGenerator(
            name = "SEQ_NODE",
            allocationSize = 1
    )
    private Long id;

    /**
     * Name of the sensor, a required value.
     */
    @Column(nullable = false)
    private String name;

    /**
     * Relationship between the sensor and a list of measurements.
     */
    @OneToMany(
            mappedBy = "nodeEntity",
            cascade = {CascadeType.MERGE},
            fetch = FetchType.LAZY
    )
    private Set<MeasurementEntity> measurements = new HashSet<>();

    /**
     * Constructor with a name value.
     * @param name
     */
    public NodeEntity(String name) {
        this.name = name;
    }

    @JsonIgnore
    public Set<MeasurementEntity> getMeasurements() {
        return measurements;
    }
}
