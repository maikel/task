FROM ubuntu:24.04

RUN apt-get update && apt-get install -y -q \
    git \
    wget \
    lsb-release \
    software-properties-common \
    gnupg \
    ninja-build

RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 19 all

RUN wget -q https://github.com/Kitware/CMake/releases/download/v3.31.3/cmake-3.31.3-linux-x86_64.sh && \
    chmod +x cmake-3.31.3-linux-x86_64.sh && \
    ./cmake-3.31.3-linux-x86_64.sh --skip-license --prefix=/usr/

