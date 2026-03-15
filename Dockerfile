# i.MX91 Cross-compiler Environment for application.
FROM ubuntu:22.04

ARG SDK_URL="https://github.com/Causality-Labs/Yocto-SDKs/releases/download/v2.0.0/fsl-framebuffer-glibc-x86_64-core-image-minimal-cortexa55-imx91frdm-toolchain-5.0.sh"

# 1. Install needed programs 
RUN apt-get update && apt-get install -y \
    make \
    openssh-client \
    python3 \
    xz-utils \
    file \
    wget \
    && rm -rf /var/lib/apt/lists/*

# 2. Retreive Yocto SDK toolchain installer
RUN wget -q ${SDK_URL} -O /tmp/sdk.sh

# 3. Deploy Yocto SDK
RUN chmod +x /tmp/sdk.sh && \
    /tmp/sdk.sh -d /opt/fsl-imx -y && \
    rm -f /tmp/sdk.sh

# 4. Automatically source the SDK for interactive shells
RUN echo "source /opt/fsl-imx/environment*" >> /etc/bash.bashrc

# 5. Set working directory
WORKDIR /build