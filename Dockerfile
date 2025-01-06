FROM silkeh/clang:19

RUN apt-get update && apt-get install -y -q \
    cppcheck \
    ninja-build \
    git

RUN wget -q https://github.com/Kitware/CMake/releases/download/v3.31.3/cmake-3.31.3-linux-x86_64.sh && \
    chmod +x cmake-3.31.3-linux-x86_64.sh && \
    ./cmake-3.31.3-linux-x86_64.sh --skip-license --prefix=/usr/

