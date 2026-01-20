#!/bin/bash

echo ""
echo "Running C++ clients..."
for i in {1..3}; do
    echo "  Client $i"
    ./bin/client
    sleep 0.5
done

echo ""
echo "Running Python client..."
python3 client.py

echo ""
echo "Testing concurrent connections..."
./bin/client & c1=$!
./bin/client & c2=$!
./bin/client & c3=$!
wait $c1 $c2 $c3

# echo ""
# echo "Stopping server..."
# kill $SERVER_PID
echo "Done!"
