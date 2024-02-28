package it.unito.sensors.server.entity;

import com.fasterxml.jackson.annotation.JsonIgnore;
import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import javax.persistence.*;

/**
 * \brief This entity represents a neighbour of another device.
 */
@Entity
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Table(name = "NEIGHBOURS")
public class NeighbourEntity {

    /**
     * Auto-generated identifier to have a unique key for this neighbour.
     */
    @Id
    @GeneratedValue
    private Long id;

    @OneToOne(cascade=CascadeType.MERGE)
    @JoinColumn(name = "device_id", referencedColumnName = "id")
    private DeviceEntity device;

    @Column(nullable = false)
    Float distance;

    @Column(nullable = false)
    Integer rssi;

    @ManyToOne
    @JoinColumn(name = "MEASUREMENT_ID", nullable = false, foreignKey = @ForeignKey(name="FK_MEASUREMENT_ID"))
    @JsonIgnore
    private MeasurementEntity measurementEntity;

}
