# CSCD58 Final Project: Load balancer with DLP integration

## Team members

Andrew & Jochebed

## Timeline

## Build instructions

**Note**: Please make sure you have the requisite libraries downloaded: `./download-libs.sh` before building.

If you need debug symbols, run:

    make debug

To generate the production executable, run:

    make production

(These commands are valid for both the load balancer daemon build process and the health status checker build process)

## Load balancer configuration

A load balancer configuration is of the form:

```
{
    "max_queued_connections": <MAX_QUEUED_CONNECTIONS>,
    "balancer_algorithm": "<BALANCER_ALGO>",
    "listener_port": <LISTENER_PORT>,
    "connection_type": "<CONNECTION_TYPE>",
    "enc_key": <ENC_KEY>,
    "health_check_interval": <HEALTH_CHECK_INTERVAL>,
    "time_until_node_down": <TIME_UNTIL_NODE_DOWN>,
    "nodes": [
        {
            "name": "Target 1",
            "weight": <WEIGHT>,
            "host", "<HOST_ADDR>",
            "target_port": <TARGET_PORT>,
            "health_daemon": <HEALTH_DAEMON_PORT>,
        },
        {
            "name": "Target 2",
            "weight": <WEIGHT>,
            "host", "<HOST_ADDR>",
            "target_port": <TARGET_PORT>,
            "health_daemon": <HEALTH_DAEMON_PORT>
        },
        {
            "name": "Target 3",
            "weight": <WEIGHT>,
            "host", "<HOST_ADDR>",
            "target_port": <TARGET_PORT>,
            "health_daemon": <HEALTH_DAEMON_PORT>
        },
        ...
    ]
}
```

The parameters are as follows:
- `<MAX_QUEUED_CONNECTIONS>`: The maximum number of queued connections before rejection.
- `<BALANCER_ALGO>`: One of the following: ROUND_ROBIN, RANDOM, RESOURCE
- `<LISTENER_PORT>`: The port to listen on
- `<CONNECTION_TYPE>`: One of the following: TCP, UDP
- `<HOST_ADDR>`: An IP address
- `<TARGET_PORT>`: The port on the IP to forward traffic to
- `<WEIGHT>`: Must be a positive integer.
- `<HEALTH_DAEMON_PORT>`: The TCP port for which the health daemon is listening on the host. Only used for the **RESOURCE** alogirithm.
- `<ENC_KEY>`: The encryption key used for securing communications between the health daemon and the load balancer. Must be a 16-byte string.
- `<HEALTH_CHECK_INTERVAL>`: The interval for which health checks are performed for nodes. Applicable for all algorithms except **RESOURCE**.
- `<TIME_UNTIL_NODE_DOWN>`: The time until a node is considered down. Only applicable for the **RESOURCE** algorithm.

Algorithms:
- **RANDOM**: Randomly assign a server to an incoming connection. Weight values act as a scalar on the probability as follows: P(choose host)=(weight/total weight).
- **ROUND_ROBIN**: Assigns servers in order. If the weights are not all equal, this changes the behaviour materially; suppose host A has weight 2 and host B has weight 1. Then, the assignment pattern is as follows: (Host A) (Host A) (Host B) (Host A) (Host A) (Host B) ...
- **RESOURCE**: Assigns servers based on load. Weights offer additional flexibility for how host traffic is directed; if host A is at 10% load and host B is at 10% load, then each would be equally likely to be chosen, but one can use the weights to make one more likely to be chosen. 

Health checks:
- Health checks use ICMP unless using the **RESOURCE** load balancing algorithm.

An example is available at `sample.json`.

## Health status daemon

The health reporter daemon can be run as follows:

    ./health_reporter <PORT> <ENC_KEY>

such that `<PORT>` is the port to listen on and `<ENC_KEY>` is an encryption key that is 16 bytes long.