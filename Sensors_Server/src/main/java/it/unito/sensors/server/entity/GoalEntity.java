package it.unito.sensors.server.entity;

import lombok.*;

import javax.persistence.*;
import java.time.LocalDateTime;

/**
 * \brief This entity represents goals managed by a node.
 *
 * A goal is identified by a unique code and has a state (REACHED, UNKNOWN etc...)
 */
@Entity(name = "GOAL")
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
@Table(name = "GOALS",
       indexes = {@Index(columnList = "CODE"), @Index(columnList = "NODE_ID"), @Index(columnList = "TIMESTAMPCREATION")},
       uniqueConstraints={@UniqueConstraint(name="UN_GOAL_CODE", columnNames={"CODE"})})
public class GoalEntity {

    /**
     * Auto-generated identifier to have a unique key for this goal.
     */
    @Id
    @GeneratedValue
    private Long id;

    @Column(nullable = false)
    private String action;

    @Column(name = "CODE", nullable = false)
    private String code;

    @Column
    private String  referenceCode;

    @Column
    private String  source;

    @Column
    private String  description;

    @Column
    private Integer priority;

    @OneToOne(fetch = FetchType.EAGER, cascade=CascadeType.MERGE)
    @JoinColumn(name = "node_id", referencedColumnName = "id")
    private NodeEntity nodeEntity;

    /**
     * Value of X-axis of the position of the goal
     */
    @Column
    private Float   posX;

    /**
     * Value of Y-axis of the position of the goal
     */
    @Column
    private Float   posY;

    /**
     * Value of the rotation in radiant of the position of the goal (see docs)
     */
    @Column
    private Float   orientW;

    @Enumerated(EnumType.ORDINAL)

    /**
     * Value of the status of the goal
     */
    private GoalStatusEnum status = GoalStatusEnum.UNKNOWN;

    /**
     * Value of the subcode of the goal
     */
    private GoalSubCodeEnum subCode = GoalSubCodeEnum.NONE;

    /**
     * It's true if the status it's a final state.
     */
    @Column
    private Boolean lock = false;

    /**
     * Creation timestamp (timestamp) of the goal.
     */
    @Column(nullable = false)
    private LocalDateTime timestampCreation;

    /**
     * Update timestamp (timestamp) of the goal.
     */
    @Column
    private LocalDateTime timestampUpdate;
}
