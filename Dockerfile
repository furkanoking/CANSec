# 1
FROM ubuntu:24.04

# 2
ENV DEBIAN_FRONTEND=noninteractive

# 3
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential cmake ninja-build gdb git pkg-config \
      linux-libc-dev can-utils iproute2 iputils-ping \
      libssl-dev openssl \
      clang lldb \
      cppcheck valgrind \
      libgtest-dev \
    && cd /usr/src/googletest && \
       cmake -S . -B build && \
       cmake --build build --config Release && \
       cp build/lib/*.a /usr/lib && \
    rm -rf /var/lib/apt/lists/*

# 4
WORKDIR /work


# from terminal write : docker build -t cansec-dev-new-thesis:latest .    We create an image file

# docker run -it --name cansec_container_thesis \
  #  --privileged --cap-add NET_ADMIN \
  #  -v "$PWD":/work -w /work \
  #  cansec-dev-new-thesis:latest bash   we create a container


# docker start cansec_container_thesis       from terminal you can start the container work

# docker exec -it cansec_container_thesis bash.  open linux terminal