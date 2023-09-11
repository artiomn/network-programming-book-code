FROM gcc:13

MAINTAINER artiomn

ENV DEBIAN_FRONTEND=noninteractive INSIDE_NB_DOCKER_CONTAINER=1

RUN apt-get update \
  && echo 'deb https://deb.debian.org/debian bookworm-backports main contrib non-free' >> /etc/apt/sources.list.d/backports.list \
#  && echo 'deb http://neilalexander.s3.eu-west-2.amazonaws.com/deb/ debian yggdrasil' >> /etc/apt/sources.list.d/yggdrasil.list \
  && apt-get update \
  && apt-get -y --no-install-recommends --show-progress install \
    apache2 \
    capnproto \
    clang \
    cmake \
    gdb \
    gdbserver \
    golang-go \
    gosu \
    inetutils-ping \
    iproute2 \
    libboost-all-dev \
    libgtest-dev \
    libglib2.0-dev \
    libgtk2.0-dev \
    libgtk-3-dev \
    libgrpc++-dev \
    libmariadb-dev \
    libmicrohttpd-dev \
    libpcap-dev \
    libplack-perl \
    libevent-2\* libevent-dev libev-dev libuv1-dev \
    libc-ares-dev \
    libpoco-dev \
    libthrift-[0-9.]\+ \
    libthrift-dev \
    libtorrent-dev \
    libtorrent-rasterbar-dev \
    liburing-dev \
    less \
    mc \
    meson \
    mpi-default-dev \
    netcat-openbsd \
    nginx \
    net-tools \
    php-common libapache2-mod-php php-cli \
    protobuf-compiler \
    protobuf-compiler-grpc \
    python3 \
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
    python3-mpi4py \
    python3-pip \
    python3-pycurl \
    python3-python-flask-jwt-extended \
    python3-pyx \
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
    tmux \
    valgrind \
    vim \
    zip \
#    yggdrasil \
  && systemctl disable apache2 \
  && systemctl disable nginx \
  && apt-get clean

#RUN \
#  git clone --recursive --depth 1 https://github.com/Kitware/CMake \
#  && cd CMake \
#  && ./bootstrap \
#  && make -j 16 \
#  && make install \
#  && cd / \
#  && rm -rf CMake

RUN \
  git clone --recursive --depth 1 https://github.com/google/flatbuffers \
  && cd flatbuffers \
  && cmake \
    -DCMAKE_BUILD_TYPE=Release -B build \
  && cmake --build build --parallel \
  && cmake --install build --prefix "/usr/local" \
  && cd / \
  && rm -rf flatbuffers \
  && git clone --recursive --depth 1 https://github.com/pistacheio/pistache \
  && cd pistache \
  && meson setup build \
    --buildtype=debug \
    -DPISTACHE_USE_SSL=true \
    -DPISTACHE_BUILD_TESTS=false \
    --prefix="/usr/local" \
  && meson compile -C build \
  && meson install -C build \
  && cd / \
  && rm -rf pistache \
  && git clone --recursive --depth 1 https://github.com/etr/libhttpserver \
  && cd libhttpserver \
  && ./bootstrap \
  && mkdir build \
  && cd build \
  && ../configure \
  && make -j16 \
  && make install \
  && cd / \
  && rm -rf libhttpserver \
  && git clone --recursive --depth 1 https://github.com/corvusoft/restbed.git \
  && cd restbed \
  && cmake -B build -DBUILD_SSL=NO -DBUILD_TESTS=NO -DCMAKE_INSTALL_PREFIX=/usr/local . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "/usr/local" \
  && cd / \
  && rm -rf restbed \
  && git clone --recursive --depth 1 https://github.com/oatpp/oatpp.git \
  && cd oatpp \
  # This is necessary to build oatpp \
  && mkdir build \
  && cd build \
  && cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DOATPP_BUILD_TESTS=OFF .. \
  && make -j32 \
  # && cmake --build . --parallel \
  && cmake --install . --prefix "/usr/local" \
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
  && pip3 install --break-system-packages scapy pythonping ping3 atheris flatbuffers gil thriftpy2 mrjob libp2p \
  && pip3 install --break-system-packages --extra-index-url "https://gitlab.com/api/v4/projects/19904939/packages/pypi/simple" pythonfuzz \
  && cd / \
  && git clone --depth 1 https://github.com/chronoxor/CppServer.git \
  && cd CppServer \
  && gil update \
  && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_POSITION_INDEPENDENT_CODE=ON -G "Ninja" -B build_linux . \
  && cmake --build build_linux --parallel \
  && cmake --install build_linux --prefix "/usr/local" \
  && mkdir /usr/local/include/CppServer \
  && cp -a modules/CppCommon/include /usr/local/include/CppCommon \
  && cp -a include/server /usr/local/include/CppServer/ \
  && find build_linux/ -iname \*.a -exec cp {} /usr/local/lib \; \
  && cp -a build_linux/cppserver-* /usr/local/bin/ \
  && cd modules/CppCommon/modules/fmt \
  && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
  && cmake --build . --parallel \
  && cmake --install .  --prefix "/usr/local" \
  && cd /CppServer/modules/asio/asio \
  && ./autogen.sh \
  && ./configure \
  && make -j16 \
  && make install \
  && cd / \
  && rm -rf CppServer \
#  && git clone --recurse-submodules --depth 1 https://github.com/matt-42/lithium.git \
#  && cd lithium \
#  && ./vcpkg/bootstrap-vcpkg.sh \
#  && vcpkg/vcpkg install \
#  && cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake . \
#  && cmake --build . --parallel \
#  && cmake --install . --prefix "/usr/local/li" \
#  && cd / \
#  && rm -rf lithium \
#  && git clone --depth 1 https://github.com/libp2p/cpp-libp2p.git \
#  && cd cpp-libp2p \
#  && cmake -DCLANG_TIDY=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -G "Ninja" -B build . \
#  && cmake --build build --parallel \
#  && cmake --install build --prefix "/usr/local" \
#  && cd / \
#  && rm -rf cpp-libp2p \
  && ldconfig

COPY ./entry.sh /
RUN chmod +x /entry.sh
COPY ./bashrc /etc/skel/.bashrc
# COPY ./yggdrasil.conf /etc/yggdrasil.conf

ENTRYPOINT ["/entry.sh"]
CMD ["/bin/bash"]
