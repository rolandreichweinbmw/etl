FROM debian:sid-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    python3 \
    python3-cogapp \
    gcc-16 \
    g++-16 \
    && rm -rf /var/lib/apt/lists/*

# gcc-snapshot
# llvm-toolchain-snapshot

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 \
    --slave /usr/bin/gcc-ar gcc-ar /usr/bin/gcc-ar-16 \
    --slave /usr/bin/gcc-nm gcc-nm /usr/bin/gcc-nm-16 \
    --slave /usr/bin/gcc-ranlib gcc-ranlib /usr/bin/gcc-ranlib-16 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-16 100

RUN useradd -m -s /bin/bash debian

WORKDIR /etl

CMD ["/bin/bash"]
