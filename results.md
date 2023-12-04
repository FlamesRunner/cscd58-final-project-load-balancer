# Load Balancer with resource monitoring

## Description

The goal of the project was to create a load balancer that supported real-time health monitoring of the nodes behind it. Indeed, implementing the random algorithm and round robin algorithms were not particularly difficult, and at that are already available in existing implementations. We did not feel that it would be sufficient to merely implement these two algorithms, so we implemented a resource-based distribution algorithm that takes into account the nodes' CPU and memory usage. It also ensures that a node that goes down is detected within a very short time frame, and so ensures a greater degree of continuity should a node go down.

## Contribution

Andrew: I implemented the base load balancer, the algorithms and the health monitoring. I also implemented several basic tests for the load balancer.
Jochebed: ...

## Instructions on how to run the load balancer

The easiest way to get up and running with the load balancer is by using the predefined Docker Compose file available at `testing/docker-compose.yml`, which will set up three web servers listening on ports 5001, 5002 and 5003. Each of the web servers will return their server name upon a well-formed HTTP request. Then, you can use the `sample.json` file to set up a basic load balancer that will use the random algorithm to distribute requests between the three web servers.

Setting up the web servers:

    cd testing
    docker-compose up -d

Compiling the load balancer:
    
    cd src-daemon
    make prod_build

Running the load balancer:

    ./load_balancer -c sample.json

Then, you can send requests to the load balancer using `curl`:

    curl localhost:5000

Additional information on modifying the load balancer configuration is available in `README.md`.

## Instructions on how to run the tests

The tests are located in the `testing` directory. It presents the option to run each test individually, or to run all of them at once with `run-all-tests.sh`. They will all automatically compile the load balancer and set up the web servers.

## Analysis and discussion of the project, results and implementation

The biggest challenge with this project was to ensure that the load balancer state was thread-safe -- this is because the project is architected in a way that separates health checking, accepting incoming connections and two threads for each connection that needs to have traffic forwarded. We dealt with this issue by making use of shared pointers, and by making sure that only the health check threads would write to the node state, while all other functions would only read from the state. In addition, we were not necessarily completely acquainted with the nuances of C++ development, which posed a challenge as we were both well-versed in C, but not in the constructs available in C++. That choice however saved significant amounts of time, for it reduced the need to write dedicated implementations of well-known data structures as in vectors (and the memory management nightmare of it), plus the somewhat more abstracted thread implementation allowed for simpler management and creation of them.

With that said, the project demonstrated the capability to handle at the very least 10 Gbit/s in traffic, which we tested by using one of the test web servers and creating a large test file. That said, we are aware of Docker's limitations for networking, so the speed could very well be higher (though the for the sake of the project, we did not investigate). 

The architecture of the load balancer roughly follows the diagram:


```
==================================
= Load balancer on TCP port 5000 =
= (Via LoadBalancer::listener)   =
= configured via json file       =
==================================
                |
                |
                |
                V
            Algorithm
    LBAlgorithm::chooseNode
                |
                |
                v
Forward traffic between the client
        and node chosen
    LoadBalancer::forward_traffic
```

Of note, once a node is chosen for an incoming connection, it remains sticky unless the node goes down, for which clients will be forced to re-connect to the load balancer and be re-assigned.

In terms of future improvements or additions, one possible addition would be to be able to use the real-time health monitoring as not only just usable for one of the algorithms, but for every algorithm as a health check and introducing additional ways to fine-tune the balancing system. Currently, only the 'resource' algorithm supports the real-time health checker, which means that the other two, 'round robin' and 'random' are relegated to using an ICMP-based check that is slower and restricted to a reduced rate.

## Lessons learned and concluding remarks

Being given this much freedom in the ability to design our projects has shown the great importance in creating good designs at the beginning, to lay down a rigid foundation on which the project will stand. Because of our choice to take care in the design of the load balancer, it does not require significant reworks to permit additional algorithms -- and because of how the health reporter is designed, we were able to add encryption to the communications between the load balancer and the health reporter via AES-128 (we are aware that AES-256 is a better choice and swapping it out would be relatively simple, but we were concerned that performance may be hindered).

That being said, we did encounter a few issues: one of the main ones being debugging the various components of the load balancer due to the (required) multi-threaded nature of the application. We ended up having to learn to use core dumps and to manually examine the calling stack to figure out some of the nastier issues, particularly as we implemented the node state (where our lack of experience in C++ very much hampered us) and attempted to make it go through C++ references. Eventually, we discovered C++ shared pointers, which ended up being exactly what we needed and saving us from needing to implement manual locking mechanisms through a Linux semaphore.