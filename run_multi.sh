#!/bin/bash

make
cd bin

echo "Starting asteroids multi..."
exec 3< <(./asteroids -fs 2>&1)

echo "Waiting for server to start..."
for i in {1..10}; do
    if read -t 1 line <&3; then
        echo "Output detected: $line"
        if [[ $line =~ "Server is on port" ]]; then
            port=$(echo $line | grep -oP 'Server is on port \K\d+')
            break
        fi
    else
        echo "Still waiting... ($i seconds)"
    fi
done

if [ -z "$port" ]; then
    echo "Failed to capture port number within 10 seconds. Server output:"
    cat <&3
    exec 3<&-
    exit 1
fi

echo "Port number is $port"

./asteroids -fp "$port"

exec 3<&-