FROM ubuntu:26.04

ENV LANG="C.UTF-8"

# Install basic build tools
RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        git \
        jq \
        vim \
        make \
        cmake \
        pkg-config \
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
        libgl1-mesa-dri \
        libdbus-1-dev \
        libudev-dev \
        libxtst-dev \
        libgtk-3-dev \
        libsdl3-dev \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean -y

# Default to zsh shell
ENV SHELL=/usr/bin/zsh \
    DOCKER_BUILDKIT=1

# Mount for docker-in-docker 
VOLUME [ "/var/lib/docker" ]

# Fire Docker/Moby script if needed
ENTRYPOINT [ "/usr/local/share/docker-init.sh", "/usr/local/share/ssh-init.sh"]
CMD [ "sleep", "infinity" ]
