package it.unito.sensors.server.entity;

import lombok.*;
import net.minidev.json.annotate.JsonIgnore;

import javax.persistence.*;
import java.util.HashSet;
import java.util.Set;

/**
 * \brief This entity represents a physical object (like clock, heart band ...) wore by a person.
 *
 */
@Entity
@AllArgsConstructor
@NoArgsConstructor
@Builder
@Getter
@Setter
@EqualsAndHashCode
@Table(name = "WEARABLES",
        indexes = @Index(columnList = "CODE"),
        uniqueConstraints={@UniqueConstraint(name="UN_WEARABLE_CODE", columnNames={"CODE"})})
public class WearableEntity {

    /**
     * Auto-generated identifier to have a unique key for this wearable.
     */
    @Id
    @GeneratedValue(
            generator = "SEQ_WEARABLE"
    )
    @SequenceGenerator(
            name = "SEQ_WEARABLE",
            allocationSize = 1
    )
    private Long id;

    /**
     * Code of the wearable, a required value.
     */
    @Column(nullable = false)
    private String code;

    /**
     * Description of the wearable, a required value.
     */
    private String description;

    /**
     * Relationship between the wearable and a list of feedbacks.
     */
    @OneToMany(
            mappedBy = "wearableEntity",
            cascade = {CascadeType.MERGE},
            fetch = FetchType.LAZY
    )
    private Set<WearableFeedbackEntity> wearableFeedbacks = new HashSet<>();

    @JsonIgnore
    public Set<WearableFeedbackEntity> getWearableFeedbacks() {
        return wearableFeedbacks;
    }

    /**
     * Constructor with a code value.
     * @param code
     */
    public WearableEntity(String code) {
        this.code = code;
    }

}
