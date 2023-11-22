#!/bin/bash
# This is a test script to check that the load balancer exits with status code 1
# when given an invalid configuration file (invalid strategy name).

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPOSITORY_ROOT_DIR="$(dirname $(dirname $(dirname ${SCRIPT_DIR})))"

# Define cleanup function
function cleanup {
    echo "> Cleaning up."
    rm ${SCRIPT_DIR}/load_balancer
}

# Start by compiling the load balancer.
echo "> Compiling the load balancer."
cd ${REPOSITORY_ROOT_DIR}/src-daemon
make production

# Start the load balancer.
echo "> Starting the load balancer."
cp ${REPOSITORY_ROOT_DIR}/src-daemon/load_balancer ${SCRIPT_DIR}
cd ${SCRIPT_DIR}
# Check that the load balancer exits with status code 1.
./load_balancer -c test-invalid-config.json > /dev/null 2>&1
if [ ! $? -eq 1 ]; then
    echo "> The exit code was not 1. Test failed."
    cleanup
    exit 1
fi

# Cleanup
cleanup

# Success.
echo "> All parts of the test passed."
exit 0