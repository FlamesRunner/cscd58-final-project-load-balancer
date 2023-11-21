# Testing

You will need some measure of a backend service to test the load balancer -- and we provide a Docker configuration file that will allow you to spawn three HTTP servers on ports 5001, 5002 and 5003. Then, you will be able to use the sample JSON file
at the repository's root and visit port 5000 to see if the behaviour is as expected.

## Running tests

### Starting Docker

Run the following to start the HTTP servers detached:

    docker-compose up -d

### Run the load balancer

Enter the `src-daemon` directory and run:

    make debug
    ./load_balancer -c ../sample.json