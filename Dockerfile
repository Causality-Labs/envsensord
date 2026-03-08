# i.MX91 Cross-compiler Environment for application.
FROM ubuntu:22.04

ARG SDK_URL="https://github.com/Causality-Labs/Yocto-SDKs/releases/download/v1.0.0/x86_64-imx91frdm-toolchain-6.6-scarthgap.tar.gz"

# 1. Install needed programs 
RUN apt-get update && apt-get install -y \
    make \
    openssh-client \
    python3 \
    xz-utils \
    file \
    wget \
    && rm -rf /var/lib/apt/lists/*

# 2. Retreive Yocto SDK toolchain (compressed)
RUN wget -q ${SDK_URL} -O /tmp/sdk.tar.gz

# 3. Deploy Yocto SDK
RUN cd /tmp && \
    tar -xzf sdk.tar.gz && \
    chmod +x fsl-imx-*.sh && \
    ./fsl-imx-*.sh -d /opt/fsl-imx -y && \
    rm -rf /tmp/sdk.tar.gz /tmp/fsl-imx*

# 4. Automatically source the SDK for interactive shells
RUN echo "source /opt/fsl-imx/environment*" >> /etc/bash.bashrc

# 5. Set working directory
WORKDIR /build