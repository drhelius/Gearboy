FROM ubuntu:24.04

ENV LANG="C.UTF-8"

# Install basic build tools
RUN apt-get update \
    && apt-get upgrade -y \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        ca-certificates \
        curl \
        git \
        jq \
        vim \
        make \
        cmake \
        unzip \
        zip \
        xxd \
        build-essential \
        gdb \
        clang \
        lldb \
        libx11-dev \
        libxext-dev \
        libxrandr-dev \
        libxcursor-dev \
        libxfixes-dev \
        libxi-dev \
        libxss-dev \
        libxkbcommon-dev \
        libwayland-dev \
        libdecor-0-dev \
        libdrm-dev \
        libgbm-dev \
        libgl1-mesa-dev \
        libegl1-mesa-dev \
        libdbus-1-dev \
        libudev-dev \
        libgtk-3-dev \
    && SDL3_TAG=$(curl -s https://api.github.com/repos/libsdl-org/SDL/releases/latest | jq -r '.tag_name') \
    && git clone --depth 1 --branch "$SDL3_TAG" https://github.com/libsdl-org/SDL.git /tmp/SDL3 \
    && cmake -S /tmp/SDL3 -B /tmp/SDL3/build -DCMAKE_INSTALL_PREFIX=/usr -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF \
    && cmake --build /tmp/SDL3/build -j"$(nproc)" \
    && cmake --install /tmp/SDL3/build \
    && rm -rf /tmp/SDL3 \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get autoremove -y && apt-get clean -y

# Default to zsh shell
ENV SHELL=/usr/bin/zsh \
    DOCKER_BUILDKIT=1

# Mount for docker-in-docker 
VOLUME [ "/var/lib/docker" ]

# Fire Docker/Moby script if needed
ENTRYPOINT [ "/usr/local/share/docker-init.sh", "/usr/local/share/ssh-init.sh"]
CMD [ "sleep", "infinity" ]
