FROM gcc:12

MAINTAINER artiomn

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update \
  && gpg --fetch-keys https://neilalexander.s3.eu-west-2.amazonaws.com/deb/key.txt \
  && gpg --export 569130E8CA20FBC4CB3FDE555898470A764B32C9 | apt-key add - \
  && echo 'deb https://deb.debian.org/debian bullseye-backports main contrib non-free' >> /etc/apt/sources.list.d/backports.list \
#  && echo 'deb http://neilalexander.s3.eu-west-2.amazonaws.com/deb/ debian yggdrasil' >> /etc/apt/sources.list.d/yggdrasil.list \
  && apt-get update \
  && apt-get -y --no-install-recommends --show-progress install \
    apache2 \
    capnproto \
    clang \
    cmake \
    gdb \
    gdbserver \
    gosu \
    inetutils-ping \
    iproute2 \
    libboost-all-dev \
    libgtest-dev \
    libglib2.0-dev \
    libgtk2.0-dev \
    libgtk-3-dev \
    libgrpc++-dev \
    libpcap-dev \
    libplack-perl \
    libevent-2\* libevent-dev libev-dev libuv1-dev \
    libc-ares-dev \
    libpoco-dev \
    libthrift-0.13.0 \
    libthrift-dev \
    libtorrent-dev \
    libtorrent-rasterbar-dev \
    less \
    meson \
    mpi-default-dev \
    netcat \
    nginx \
    net-tools \
    php-common libapache2-mod-php php-cli \
    protobuf-compiler \
    protobuf-compiler-grpc \
    python \
    python3-cryptography \
    python3-django \
    python3-fastapi \
    python3-flask \
    python3-flask-api \
    python3-flasgger \
    python3-flask-restful \
    python3-flask-sockets \
    python3-flatbuffers \
    python3-grpcio \
    python3-matplotlib \
    python3-pip \
    python3-python-flask-jwt-extended \
    python3-pyx \
    python3-scapy \
    python3-thrift \
    qt6-base-dev \
    rapidjson-dev \
    rsync \
    ruby-rack \
    sudo \
    ssh \
    tar \
    telnet \
    texlive-latex-base \
    tcpdump \
    thrift-compiler \
    valgrind \
    vim \
#    yggdrasil \
  && systemctl disable apache2 \
  && systemctl disable nginx \
  && git clone --recursive --depth 1 https://github.com/google/flatbuffers \
  && cd flatbuffers \
  && cmake \
    -DCMAKE_BUILD_TYPE=Release -B build \
  && cmake --build build --parallel \
  && cmake --install build --prefix "/usr/local" \
  && cd / \
  && rm -rf flatbuffers \
  && git clone --recursive --depth 1 https://github.com/pistacheio/pistache \
  && cd pistache \
#  && cmake \
#    -DPISTACHE_USE_SSL=true \
#    -DPISTACHE_BUILD_TESTS=false \
#    -DCMAKE_INSTALL_PREFIX=/usr/local \
#    . \
  && meson setup build \
    --buildtype=debug \
    -DPISTACHE_USE_SSL=true \
    -DPISTACHE_BUILD_TESTS=false \
    --prefix="/usr/local" \
  && meson compile -C build \
  && meson install -C build \
  && cd / \
  && rm -rf pistache \
  && git clone --recursive --depth 1 https://github.com/corvusoft/restbed.git \
  && cd restbed \
  && cmake -B build -DBUILD_SSL=YES -DBUILD_TESTS=NO -DCMAKE_INSTALL_PREFIX=/usr/local . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "/usr/local" \
  && cd / \
  && rm -rf restbed \
  && git clone --recursive --depth 1 https://github.com/oatpp/oatpp.git \
  && cd oatpp \
  && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "/usr/local" \
  && cd / \
  && rm -rf oatpp \
  && git clone --recursive --depth 1 https://github.com/oatpp/oatpp-websocket.git \
  && cd oatpp-websocket \
  && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "/usr/local" \
  && cd / \
  && rm -rf oatpp-websocket \
  && git clone --recursive --depth 1 https://github.com/oatpp/oatpp-swagger.git \
  && cd oatpp-swagger \
  && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "/usr/local" \
  && cd / \
  && rm -rf oatpp-swagger \
  && git clone --recursive --depth 1 https://github.com/oatpp/oatpp-curl.git \
  && cd oatpp-curl \
  && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "/usr/local" \
  && cd / \
  && rm -rf oatpp-curl \
  && git clone --recursive --depth 1 https://github.com/Thalhammer/jwt-cpp \
  && cd jwt-cpp \
  && cmake -B build -DBUILD_SSL=YES -DBUILD_TESTS=NO -DCMAKE_INSTALL_PREFIX=/usr/local . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "/usr/local" \
  && rm -rf jwt-cpp \
  && cd / \
  && git clone --recursive --depth 1 https://github.com/reo7sp/tgbot-cpp \
  && cd tgbot-cpp \
  && cmake . \
  && cmake --build . --parallel \
  && cmake --install . --prefix "/usr/local" \
  && cd / \
  && rm -rf tgbot-cpp \
  && pip3 install thriftpy2 flatbuffers \
  && ldconfig \
  && apt-get clean

COPY ./entry.sh /
RUN chmod +x /entry.sh
COPY ./bashrc /etc/skel/.bashrc
# COPY ./yggdrasil.conf /etc/yggdrasil.conf

ENTRYPOINT ["/entry.sh"]
CMD ["/bin/bash"]
