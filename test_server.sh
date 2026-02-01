#!/bin/bash

# Environmental Sensor Server Test Script
# Tests server with concurrent client connections

# Configuration
NUM_CLIENTS=${1:-5}  # Number of clients per test (default: 5)
CLIENT_TYPE=${2:-both}  # Which client to test: cpp, python, or both (default: both)

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Request types for variety
REQUESTS=("-a" "-t" "-u" "-r" "-t -u" "-t -r" "-u -r")

# Function to print usage
print_usage() {
    echo "Usage: $0 [NUM_CLIENTS] [CLIENT_TYPE]"
    echo ""
    echo "Environmental Sensor Server Test Script"
    echo "Tests server with concurrent client connections"
    echo ""
    echo "Arguments:"
    echo "  NUM_CLIENTS    Number of concurrent clients (default: 5)"
    echo "  CLIENT_TYPE    Which client to test: cpp, python, or both (default: both)"
    echo ""
    echo "Options:"
    echo "  -h, --help     Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0              # Test both clients with 5 instances each"
    echo "  $0 20           # Test both clients with 20 instances each"
    echo "  $0 10 cpp       # Test only C++ client with 10 instances"
    echo "  $0 15 python    # Test only Python client with 15 instances"
    echo ""
}

# Check for help flag
if [[ "$1" == "-h" ]] || [[ "$1" == "--help" ]]; then
    print_usage
    exit 0
fi

# Validate client type
case "$CLIENT_TYPE" in
    cpp|python|both)
        ;;
    *)
        echo -e "${RED}Error: Invalid client type '$CLIENT_TYPE'${NC}"
        echo ""
        print_usage
        exit 1
        ;;
esac

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Environmental Sensor Server Test${NC}"
echo -e "${BLUE}========================================${NC}"
echo -e "Testing with ${GREEN}$NUM_CLIENTS${NC} concurrent clients per test"
echo -e "Client type: ${GREEN}$CLIENT_TYPE${NC}"
echo ""

# Function to run test
run_test() {
    local client_cmd="$1"
    local test_name="$2"
    local pids=()
    local success=0
    local failure=0
    
    echo -e "${YELLOW}Testing $test_name...${NC}"
    
    # Create log directory
    mkdir -p logs
    local log_dir="logs/test_$(date +%Y%m%d_%H%M%S)"
    mkdir -p "$log_dir"
    
    # Start timer
    local start_time=$(date +%s.%N)
    
    # Launch clients
    for i in $(seq 1 $NUM_CLIENTS); do
        # Vary requests for more realistic testing
        local req_args="${REQUESTS[$((i % ${#REQUESTS[@]}))]}"
        echo "  Client $i (requesting: $req_args)"
        
        if [[ "$client_cmd" == *"python"* ]]; then
            $client_cmd $req_args > "$log_dir/client_$i.log" 2>&1 &
        else
            $client_cmd $req_args > "$log_dir/client_$i.log" 2>&1 &
        fi
        
        pids+=($!)
        sleep 0.05  # Small stagger to simulate realistic traffic
    done
    
    echo "  Waiting for clients to complete..."
    
    # Wait for all and track results
    for pid in "${pids[@]}"; do
        if wait $pid; then
            ((success++))
        else
            ((failure++))
        fi
    done
    
    # End timer
    local end_time=$(date +%s.%N)
    local duration=$(echo "$end_time - $start_time" | bc)
    local req_per_sec=$(echo "scale=2; $NUM_CLIENTS / $duration" | bc)
    
    # Display results
    echo ""
    echo -e "${BLUE}Results for $test_name:${NC}"
    echo -e "  Total:      $NUM_CLIENTS"
    echo -e "  Success:    ${GREEN}$success${NC}"
    echo -e "  Failed:     $([ $failure -eq 0 ] && echo -e ${GREEN} || echo -e ${RED})$failure${NC}"
    echo -e "  Duration:   ${duration}s"
    echo -e "  Rate:       ${req_per_sec} req/s"
    echo -e "  Logs:       $log_dir/"
    
    # Show failed client logs if any
    if [ $failure -gt 0 ]; then
        echo ""
        echo -e "${RED}Failed client outputs:${NC}"
        for i in $(seq 1 $NUM_CLIENTS); do
            if ! grep -q "Disconnected from server" "$log_dir/client_$i.log" 2>/dev/null && \
               ! grep -q "Server response:" "$log_dir/client_$i.log" 2>/dev/null; then
                echo -e "${RED}  Client $i:${NC}"
                cat "$log_dir/client_$i.log" | sed 's/^/    /'
            fi
        done
    fi
    
    echo ""
    return $failure
}

# Check if clients exist and run tests
total_failures=0

if [ "$CLIENT_TYPE" = "cpp" ] || [ "$CLIENT_TYPE" = "both" ]; then
    if [ ! -f "./client" ] && [ ! -f "./bin/client" ]; then
        echo -e "${RED}Error: C++ client not found. Skipping C++ test.${NC}"
        echo -e "${YELLOW}Compile with 'make client' first.${NC}"
        echo ""
    else
        CLIENT_BIN="./client"
        [ -f "./bin/client" ] && CLIENT_BIN="./bin/client"
        
        run_test "$CLIENT_BIN" "C++ Client"
        ((total_failures+=$?))
    fi
fi

if [ "$CLIENT_TYPE" = "python" ] || [ "$CLIENT_TYPE" = "both" ]; then
    if [ ! -f "envSensorClient.py" ]; then
        echo -e "${RED}Error: Python client not found. Skipping Python test.${NC}"
        echo ""
    else
        run_test "python3 envSensorClient.py" "Python Client"
        ((total_failures+=$?))
    fi
fi

# Final summary
echo -e "${BLUE}========================================${NC}"
if [ $total_failures -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}✗ $total_failures client(s) failed${NC}"
    exit 1
fi
