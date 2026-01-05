# Cross-compile for BeagleBone Black (ARMv7)
FROM debian:trixie-slim

# Install cross-compilation toolchain
RUN apt-get update && apt-get install -y \
    g++-arm-linux-gnueabihf \
    make \
    openssh-client \
    device-tree-compiler \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

