#!/bin/bash
# This is a test script for the random strategy.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPOSITORY_ROOT_DIR="$(dirname $(dirname $(dirname ${SCRIPT_DIR})))"

# Start by compiling the load balancer.
echo "> Compiling the load balancer."
cd ${REPOSITORY_ROOT_DIR}/src-daemon
make production

# Start the load balancer.
echo "> Starting the load balancer."
cp ${REPOSITORY_ROOT_DIR}/src-daemon/load_balancer ${SCRIPT_DIR}
cd ${SCRIPT_DIR}
./load_balancer -c test-random-standard.json > /dev/null 2>&1 &

# docker-compose up -d
echo "> Starting the servers."
cd ${REPOSITORY_ROOT_DIR}/testing
docker-compose up -d &> /dev/null

# Attempt to download three times from localhost:5000 and check that the
# requests are being forwarded to some server.
cd ${SCRIPT_DIR}
for i in {1..3}
do
    curl -s localhost:5000 > curl.out
    if grep -q "This file is being served from server" curl.out; then
        echo "> Part ${i} of the test passed."
    else
        echo "> Part ${i} of the test failed."
        exit 1
    fi
done

# docker-compose down
echo "> Cleaning up."
cd ${REPOSITORY_ROOT_DIR}/testing
docker-compose down &> /dev/null
rm ${SCRIPT_DIR}/load_balancer
rm ${SCRIPT_DIR}/curl.out

# Kill the load balancer.
kill $(ps aux | grep '[l]oad_balancer' | awk '{print $2}')

# Success.
echo "> All parts of the test passed."
exit 0