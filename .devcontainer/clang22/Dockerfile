FROM debian:sid-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    python3 \
    python3-cogapp \
    clang-22 \
    clang++-22 \
    llvm-22 \
    libc++-22-dev \
    libc++abi-22-dev \
    libc++-22-dev \
    && rm -rf /var/lib/apt/lists/*

# gcc-snapshot
# llvm-toolchain-snapshot

RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-22 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-22 100 \
    && update-alternatives --install /usr/bin/llvm-ar llvm-ar /usr/bin/llvm-ar-22 100 \
    --slave /usr/bin/llvm-nm llvm-nm /usr/bin/llvm-nm-22 \
    --slave /usr/bin/llvm-ranlib llvm-ranlib /usr/bin/llvm-ranlib-22

RUN useradd -m -s /bin/bash debian

WORKDIR /etl

CMD ["/bin/bash"]
