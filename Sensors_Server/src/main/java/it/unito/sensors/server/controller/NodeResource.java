package it.unito.sensors.server.controller;

import it.unito.sensors.server.entity.NodeEntity;
import it.unito.sensors.server.repository.NodeRepository;
import java.util.List;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

/**
 * \brief REST API to handle physical nodes (robots, rover, drones...)
 */
@RestController
public class NodeResource {

	@Autowired
    private NodeRepository nodeRepository;

    @GetMapping(
            value = "/node"
    )
    public List<NodeEntity> retrieveAllNodes() {
        return nodeRepository.findAll();
    }

    @GetMapping(
            value = "/node/{id}",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public NodeEntity retrieveNode(@PathVariable long id) {
        return nodeRepository.findById(id);
    }

    @PostMapping(
            value = "/node",
            produces = {MediaType.APPLICATION_JSON_VALUE}
    )
    public ResponseEntity<NodeEntity> createNode(@RequestParam String name) {
        NodeEntity nodeEntityToFind = nodeRepository.findByName(name);

        if (nodeEntityToFind != null) {
            return ResponseEntity.status(HttpStatus.BAD_REQUEST).build();
        }

        NodeEntity nodeEntity = new NodeEntity(name);
        nodeRepository.save(nodeEntity);
        return ResponseEntity.ok(nodeEntity);
    }
}