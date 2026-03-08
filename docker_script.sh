#!/bin/bash

# Docker script for cross-compilation environment

function show_help {
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  -b, --build    Build the Docker container"
    echo "  -r, --run      Run the container interactively"
    echo "  -h, --help     Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --build"
    echo "  $0 -r"
}

function build_container {
    echo "Building Docker container 'imx91-cross-build'..."
    docker build -t imx91-cross-build .
    
    if [ $? -eq 0 ]; then
        echo "✓ Container built successfully"
    else
        echo "✗ Container build failed"
        exit 1
    fi
}

function run_container {
    echo "Starting Docker container..."
    docker run --rm -it --network=host \
        -v $(pwd):/build \
        imx91-cross-build /bin/bash
}

# Main script
case "$1" in
    -b|--build)
        build_container
        ;;
    -r|--run)
        run_container
        ;;
    -h|--help|"")
        show_help
        ;;
    *)
        echo "Error: Unknown option '$1'"
        echo ""
        show_help
        exit 1
        ;;
esac