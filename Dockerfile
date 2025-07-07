FROM ubuntu:20.04

ARG ESP8266_RTOS_SDK_VERSION=v3.4
ARG XTENSA_LX106_RELEASE=xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64

ENV PATH=/opt/toolchains/lx106/bin:$PATH
ENV IDF_PATH=/opt/sdk
ENV PATH=/opt/sdk/tools:$PATH

# Set non-interactive installation and timezone
ENV DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC

RUN apt-get update && apt-get install -y \
        git \
        ca-certificates \
        curl \
        tar \
        gcc \
        wget \
        make \
        libncurses-dev \
        flex \
        bison \
        gperf \
        python3-serial \
        python3-pip \
        cmake \
        lsof \
    && update-alternatives --install /usr/bin/python python /usr/bin/python3 10 \
    && update-alternatives --install /usr/bin/pip pip /usr/bin/pip3 10

# Download and extract the toolchain
RUN curl -L https://dl.espressif.com/dl/${XTENSA_LX106_RELEASE}.tar.gz -o /tmp/xtensa-lx106.tar.gz \
    && mkdir -p /opt/toolchains \
    && tar -xzf /tmp/xtensa-lx106.tar.gz -C /opt/toolchains \
    && mv /opt/toolchains/xtensa-lx106-elf /opt/toolchains/lx106 \
    && rm /tmp/xtensa-lx106.tar.gz

# Clone the SDK
RUN mkdir -p /opt/sdk \
    && for i in 1 2 3; do \
         git clone --recursive -b ${ESP8266_RTOS_SDK_VERSION} https://github.com/espressif/ESP8266_RTOS_SDK.git /opt/sdk && break || sleep 5; \
       done

# Install Python dependencies of the SDK
RUN pip install --no-cache-dir --upgrade pip \
    && pip install --no-cache-dir -r /opt/sdk/requirements.txt

# Clone and build mkspiffs with full flags
RUN git clone https://github.com/igrr/mkspiffs.git /opt/mkspiffs \
    && cd /opt/mkspiffs \
    && git submodule update --init --recursive \
    && make dist BUILD_CONFIG_NAME=-esp8266rtos \
        CPPFLAGS="\
        -DSPIFFS_OBJ_META_LEN=4 \
        -DSPIFFS_USE_MAGIC=1 \
        -DSPIFFS_USE_MAGIC_LENGTH=1 \
        -DSPIFFS_ALIGNED_OBJECT_INDEX_TABLES=1 \
        -DSPIFFS_OBJ_NAME_LEN=32"

# Add mkspiffs to PATH
ENV PATH="/opt/mkspiffs:${PATH}"

# Set environment variables for SPIFFS
ENV SPIFFS_BASE_ADDR=0x210000
ENV SPIFFS_SIZE=0xF0000

LABEL ESP8266_RTOS_SDK_VERSION=$ESP8266_RTOS_SDK_VERSION \
      XTENSA_LX106_RELEASE=$XTENSA_LX106_RELEASE

CMD ["bash"]