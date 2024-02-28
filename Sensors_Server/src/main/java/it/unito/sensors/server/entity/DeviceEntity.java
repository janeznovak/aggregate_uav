package it.unito.sensors.server.entity;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import javax.persistence.*;

/**
 * \brief This entity represents physical devices identified by mac address and unique local name.
 *
 * A device is in the real world and shares the position with other devices.
 */
@Entity
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Table(name = "DEVICES",
       indexes = @Index(columnList = "MAC_ADDRESS"),
       uniqueConstraints={@UniqueConstraint(name="UN_MAC_ADDRESS", columnNames={"MAC_ADDRESS"})})
public class DeviceEntity {

    /**
     * Auto-generated identifier to have a unique key for this device.
     */
    @Id
    @GeneratedValue
    private Long id;

    @Column(name = "LOCAL_NAME", nullable = false)
    String localName;

    @Column(name = "MAC_ADDRESS", nullable = false)
    String macAddress;
}
