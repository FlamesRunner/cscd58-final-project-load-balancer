#!/bin/bash
# For each directory in testing/tests, run the run.sh script and check that it
# exits with status code 0.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPOSITORY_ROOT_DIR="$(dirname $(dirname ${SCRIPT_DIR}))"

for test_dir in ${REPOSITORY_ROOT_DIR}/testing/tests/*/
do
    echo "> Running test in ${test_dir}."
    cd ${test_dir}
    ./run.sh
    if [ ! $? -eq 0 ]; then
        echo "> One or more tests have failed."
        exit 1
    fi
done

echo "> All tests have passed."
exit 0