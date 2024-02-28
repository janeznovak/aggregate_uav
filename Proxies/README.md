# Install 

```bash
rosdep install --from-paths src --ignore-src -r -y -i

# Import distribution dependencies
colcon build --event-handlers console_direct+ --symlink-install
```

# Run
## Embedded mode
Already included in create3 launchers.

## Server mode
Server proxies that read/send data from/to deployed instances of embedded proxies.
```bash
cd PoC
./server_proxies_run.bash
```