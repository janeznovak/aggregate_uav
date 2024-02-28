package it.unito.sensors.server.entity;

import lombok.*;

import javax.persistence.*;

/**
 * \brief This entity represents the code of the object that can be searched in the world (like in the library).
 */
@Entity
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@ToString
@Table(name = "ITEMS",
       uniqueConstraints={@UniqueConstraint(name="UN_ITEM_CODE", columnNames={"CODE"})})
public class ItemEntity {

    /**
     * Auto-generated identifier to have a unique key for this item.
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
    private Float   orientW;
}
