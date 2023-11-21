#!/bin/bash
# Downloads the requisite libraries for the load balancer.

if [ -z $(which git) ]; then
    echo "Error: Please make sure you have Git installed and on your PATH variable."
fi

# Cleanup
rm -rf src-daemon/json

# Download
mkdir tmp-lib
cd tmp-lib
git clone git@github.com:nlohmann/json.git
cp -R json/single_include/nlohmann/json.hpp ../src-daemon/hdrs/json.hpp
cd ..
rm -rf tmp-lib

echo "Done"