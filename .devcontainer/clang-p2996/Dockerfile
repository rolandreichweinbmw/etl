FROM vsavkov/clang-p2996:amd64

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    ninja-build \
    python3 \
    python3-cogapp \
    && rm -rf /var/lib/apt/lists/*

RUN echo "/opt/p2996/clang/lib/x86_64-unknown-linux-gnu" | tee /etc/ld.so.conf.d/p2996.conf

RUN ldconfig

WORKDIR /etl

CMD ["/bin/bash"]
