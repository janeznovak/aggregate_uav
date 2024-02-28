package it.unito.sensors.server.entity;

import lombok.*;

import javax.persistence.*;
import java.time.LocalDateTime;

/**
 * \brief This entity represents feedback, like positions or body temperature, coming from different wearables.
 */
@Entity
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@ToString
@Builder
@Table(name = "WEARABLE_FEEDBACKS")
public class WearableFeedbackEntity {

    /**
     * Auto-generated identifier to have a unique key for this feedback.
     */
    @Id
    @GeneratedValue
    private Long id;

    @Column(name = "CODE", nullable = false)
    String code;

    @Column
    private Float   posX;

    @Column
    private Float   posY;

    @Column
    private Integer   heartRatePerMinute;

    @Column
    private Float   bodyTemperature;

    @Column
    private Integer   oxygenation;

    @Enumerated(EnumType.ORDINAL)
    private GoalStatusEnum goalStatus = GoalStatusEnum.NO_GOAL;

    /**
     * Timestamp (unix timestamp, milliseconds) of the feedback.
     */
    @Column(nullable = false)
    private Long unixTimestamp;

    /**
     * Timestamp (timestamp) of the feedback.
     */
    @Column(nullable = false)
    private LocalDateTime timestamp;

    @OneToOne(fetch = FetchType.EAGER, cascade=CascadeType.MERGE)
    @JoinColumn(name = "wearable_id", referencedColumnName = "id")
    private WearableEntity wearableEntity;
}