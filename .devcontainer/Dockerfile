#-----------------------------------------------------------------------------
# Base image for ESP32 development with PlatformIO and Dev Containers
#-----------------------------------------------------------------------------
# Uses the official devcontainers Python base for vscode user, sudo, and
# common dev tooling, then adds ESP32 toolchains, PlatformIO, and ESP-IDF.
#-----------------------------------------------------------------------------
FROM mcr.microsoft.com/devcontainers/python:3.12-bookworm

#-----------------------------------------------------------------------------
# Build arguments
#-----------------------------------------------------------------------------
ARG ESP_IDF_VERSION=v5.2

#-----------------------------------------------------------------------------
# Environment
#-----------------------------------------------------------------------------
ENV DEBIAN_FRONTEND=noninteractive
ENV IDF_PATH=/opt/esp-idf
ENV PLATFORMIO_CORE_DIR=/home/vscode/.platformio
# Keep both pip user installs and system paths
ENV PATH="/home/vscode/.local/bin:/home/vscode/.platformio/penv/bin:${PATH}"

#-----------------------------------------------------------------------------
# System dependencies for ESP32 development
#-----------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake \
    ninja-build \
    pkg-config \
    gcc \
    g++ \
    make \
    dfu-util \
    libusb-1.0-0 \
    udev \
    && rm -rf /var/lib/apt/lists/*

#-----------------------------------------------------------------------------
# PlatformIO Core (installed globally via pip)
#-----------------------------------------------------------------------------
RUN pip install --no-cache-dir --upgrade pip \
    && pip install --no-cache-dir platformio pyserial \
    && mkdir -p ${PLATFORMIO_CORE_DIR} \
    # Pre-install the espressif32 platform so first build is faster
    && platformio platform install espressif32 \
    # Install udev rules for USB/serial device access
    && platformio system --udev-rules 2>/dev/null || true

#-----------------------------------------------------------------------------
# ESP-IDF (optional, for native ESP-IDF framework development)
#-----------------------------------------------------------------------------
RUN git clone --depth 1 --branch ${ESP_IDF_VERSION} \
    --recursive https://github.com/espressif/esp-idf.git ${IDF_PATH} \
    && cd ${IDF_PATH} \
    && ./install.sh \
    && echo "source ${IDF_PATH}/export.sh" >> /home/vscode/.bashrc

#-----------------------------------------------------------------------------
# Python testing and utility packages
#-----------------------------------------------------------------------------
RUN pip install --no-cache-dir pytest pytest-xdist pytest-mock

#-----------------------------------------------------------------------------
# Ensure PlatformIO cache directory and home are owned by vscode user.
# Also give vscode access to ESP-IDF toolchain.
#-----------------------------------------------------------------------------
RUN chown -R vscode:vscode /home/vscode/.platformio 2>/dev/null || true \
    && chown -R vscode:vscode /home/vscode/.espressif 2>/dev/null || true \
    && chown -R vscode:vscode /opt/esp-idf 2>/dev/null || true

#-----------------------------------------------------------------------------
# Default working directory (overridden by devcontainer.json)
#-----------------------------------------------------------------------------
WORKDIR /workspace
