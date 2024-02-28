package it.unito.sensors.server.entity;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import javax.persistence.*;
import java.util.List;

@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Embeddable
public class MeasurementData {

    @Column(nullable = false)
    private Float temperature;

    @Column(nullable = false)
    private Float pressure;

    @Column(nullable = false)
    private Float humidity;

    /* magnetometer */
    @Column(nullable = false) //mGauss
    private Integer magneticInductionX;

    @Column(nullable = false) //mGauss
    private Integer magneticInductionY;

    @Column(nullable = false) //mGauss
    private Integer magneticInductionZ;

    /* accelerometer */
    @Column(nullable = false) //mg
    private Float accelerationX;

    @Column(nullable = false) //mg
    private Float accelerationY;

    @Column(nullable = false) //mg
    private Float accelerationZ;

    /* gyroscope */
    @Column(nullable = false) //mdps
    private Integer rotationX;

    @Column(nullable = false) //mdps
    private Integer rotationY;

    @Column(nullable = false) //mdps
    private Integer rotationZ;
}
