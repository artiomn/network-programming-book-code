FROM gcc:13

MAINTAINER artiomn

ENV DEBIAN_FRONTEND=noninteractive INSIDE_NB_DOCKER_CONTAINER=1 \
  GIT_CLONE_PARAMETERS="--recursive --depth 1 -j 16 --single-branch" \
  INSTALL_PREFIX="/usr/local"

WORKDIR /

RUN \
  echo 'deb https://deb.debian.org/debian bookworm-backports main contrib non-free' >> /etc/apt/sources.list.d/backports.list \
  && echo 'deb-src https://deb.debian.org/debian bookworm-backports main contrib non-free' >> /etc/apt/sources.list.d/backports.list \
#  && echo 'deb http://neilalexander.s3.eu-west-2.amazonaws.com/deb/ debian yggdrasil' >> /etc/apt/sources.list.d/yggdrasil.list \
  && apt-get update \
  && apt-get -y --no-install-recommends --show-progress install \
    # For MR4C \
    ant \
    apache2 \
    capnproto \
    clang \
    cmake \
    cppzmq-dev \
    dpdk-dev \
    erlang \
    gdb \
    gdbserver \
    golang-go \
    gosu \
    gunicorn \
    inetutils-ping \
    iproute2 \
    libboost-all-dev \
    libc-ares-dev \
    libcapnp-dev \
    libcppunit-dev \
    libgdal-dev \
    libevent-2\* libevent-dev libev-dev libuv1-dev \
    libglib2.0-dev \
    libgtest-dev \
    libgtk2.0-dev \
    libgtk-3-dev \
    libgrpc++-dev \
    liblog4cxx-dev \
    libmariadb-dev \
    libmicrohttpd-dev \
    libmodbus-dev \
    libmosquitto-dev \
    libnats-dev \
    libpcap-dev \
    libplack-perl \
    libpoco-dev \
    libprotobuf-dev \
    libprotobuf-c-dev \
    libqpid-proton-cpp12-dev \
    librdkafka-dev \
    libsdl2-net-dev \
    libthrift-[0-9.]\+ \
    libthrift-dev \
    libtorrent-dev \
    libtorrent-rasterbar-dev \
    liburing-dev \
    libzmq3-dev \
    less \
    mc \
    meson \
    mpi-default-dev \
    netcat-openbsd \
    nginx \
    net-tools \
    npm \
    php-common libapache2-mod-php php-cli \
    protobuf-compiler \
    protobuf-compiler-grpc \
    pyqt6-dev \
    python3 \
    python3-aiohttp \
    python3-anyio \
    python3-confluent-kafka \
    python3-cryptography \
    python3-django \
    python3-fastapi \
    python3-flask \
    python3-flask-api \
    python3-flasgger \
    python3-flask-restful \
    python3-flask-sockets \
    python3-flatbuffers \
    python3-gevent \
    python3-gi \
    python3-grpcio \
    python3-hypercorn \
    python3-httplib2 \
    python3-kafka \
    python3-libtorrent \
    python3-matplotlib \
    python3-mpi4py \
    python3-paho-mqtt \
    python3-pika \
    python3-pip \
    python3-pycurl \
    python3-pymodbus \
    python3-python-flask-jwt-extended \
    python3-pyx \
    python3-pyqt6 \
    python3-pyqt6.qtwebsockets \
    python3-requests \
    python3-sniffio \
    python3-thrift \
    python3-tornado \
    python3-trio \
    python3-urllib3 \
    python3-uvloop \
    python3-zmq \
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
    tshark \
    uvicorn \
    uwsgi \
    valgrind \
    vim \
    wfuzz \
    wireshark-qt \
    zip \
#    yggdrasil \
  && systemctl disable apache2 \
  && systemctl disable nginx \
  && apt-get clean

COPY ././.gitconfig /etc/skel/
COPY ./patches /tmp/

RUN \
    # Need for the iproute2 package build: \
    apt-get update \
    && apt-get -y --no-install-recommends --show-progress install \
        build-essential:native build-essential:native bison debhelper-compat dh-package-notes flex libxtables-dev libatm1-dev libcap-dev po-debconf dialog \
    && mkdir /tmp/iproute_install \
    && cd /tmp/iproute_install  \
    && apt-get source iproute2 \
    && cd iproute2-6.7.0/ \
    && sed -i -e '1 i\include/* /usr/include/iproute2/' -e '$a\lib/*.a /usr/lib/${DEB_HOST_MULTIARCH}/' debian/iproute2.install \
    && cp /tmp/patches/0003-librtnetnlink-add.patch debian/patches \
    && echo "0003-librtnetnlink-add.patch" >>  debian/patches/series \
    && dpkg-buildpackage -us -uc \
    && DEBIAN_FRONTEND=noninteractive dpkg -i ../iproute2_6.7.0-2~bpo12+1_amd64.deb \
    && cd / \
    && rm -rf /tmp/iproute_install \
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
  git clone ${GIT_CLONE_PARAMETERS} https://github.com/uNetworking/uWebSockets \
  && cd uWebSockets \
  && CXXFLAGS="${CXXFLAGS} -fPIE" CFLAGS="${CFLAGS} -fPIE" make WITH_LTO=0 WITH_LIBUV=1 -j16 examples \
  && make prefix=${INSTALL_PREFIX} install \
  && install uSockets/uSockets.a ${INSTALL_PREFIX}/lib/libusockets.a \
  && install uSockets/src/libusockets.h ${INSTALL_PREFIX}/include/ \
  && cd / \
  && rm -rf uWebSockets \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/google/flatbuffers \
  && cd flatbuffers \
  && cmake \
    -DCMAKE_BUILD_TYPE=Release -B build \
  && cmake --build build --parallel \
  && cmake --install build --prefix "${INSTALL_PREFIX}" \
  && cd / \
  && rm -rf flatbuffers \
  # Build only gRPC console \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/grpc/grpc \
  && cd grpc \
  && cmake -B build -DgRPC_BUILD_TESTS=ON -DgRPC_INSTALL=ON -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} . \
  && cmake --build build --target grpc_cli \
  && mv build/grpc_cli ${INSTALL_PREFIX}/bin/ \
  && cd / \
  && rm -rf grpc \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/pistacheio/pistache \
  && cd pistache \
  && meson setup build \
    --buildtype=debug \
    -DPISTACHE_USE_SSL=true \
    -DPISTACHE_BUILD_TESTS=false \
    --prefix="${INSTALL_PREFIX}" \
  && meson compile -C build \
  && meson install -C build \
  && cd / \
  && rm -rf pistache \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/etr/libhttpserver \
  && cd libhttpserver \
  && ./bootstrap \
  && mkdir build \
  && cd build \
  && ../configure \
  && make -j16 \
  && make install \
  && cd / \
  && rm -rf libhttpserver \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/corvusoft/restbed.git \
  && cd restbed \
  && cmake -B build -DBUILD_SSL=NO -DBUILD_TESTS=NO -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "${INSTALL_PREFIX}" \
  && cd / \
  && rm -rf restbed \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/nodejs/llhttp \
  && cd llhttp \
  && npm install -D @types/node --force --legacy-peer-deps \
  && echo y|make RELEASE=8.0 release \
  && cd release \
  && cmake -B build -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_STATIC_LIBS=ON -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "${INSTALL_PREFIX}" \
  && cd / \
  && rm -rf llhttp \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/oatpp/oatpp.git \
  && cd oatpp \
  # This is necessary to build oatpp \
  && mkdir build \
  && cd build \
  && cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DOATPP_BUILD_TESTS=OFF .. \
  && make -j32 \
  # && cmake --build . --parallel \
  && cmake --install . --prefix "${INSTALL_PREFIX}" \
  && cd / \
  && rm -rf oatpp \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/oatpp/oatpp-websocket.git \
  && cd oatpp-websocket \
  && cmake -B build -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DOATPP_BUILD_TESTS=OFF . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "${INSTALL_PREFIX}" \
  && cd / \
  && rm -rf oatpp-websocket \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/oatpp/oatpp-swagger.git \
  && cd oatpp-swagger \
  && cmake -B build -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DOATPP_BUILD_TESTS=OFF . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "${INSTALL_PREFIX}" \
  && cd / \
  && rm -rf oatpp-swagger \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/oatpp/oatpp-curl.git \
  && cd oatpp-curl \
  && cmake -B build -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DOATPP_BUILD_TESTS=OFF . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "${INSTALL_PREFIX}" \
  && cd / \
  && rm -rf oatpp-curl \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/Thalhammer/jwt-cpp \
  && cd jwt-cpp \
  && cmake -B build -DJWT_BUILD_TESTS=NO -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "${INSTALL_PREFIX}" \
  && rm -rf jwt-cpp \
  && cd / \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/reo7sp/tgbot-cpp \
  && cd tgbot-cpp \
  && cmake -B build -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DENABLE_TESTS=OFF . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "${INSTALL_PREFIX}" \
  && cd / \
  && rm -rf tgbot-cpp \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/mfontanini/cppkafka \
  && cd cppkafka \
  && cmake -B build -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DCPPKAFKA_BUILD_SHARED=0 -DCPPKAFKA_DISABLE_TESTS=ON -DCPPKAFKA_DISABLE_EXAMPLES=ON . \
  && cmake --build build --parallel \
  && cmake --install build --prefix "${INSTALL_PREFIX}" \
  && cd / \
  && rm -rf cppkafka \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/google/mr4c \
  && cd mr4c \
  && git apply /tmp/patches/mr4c.patch \
  && ./build_all \
  && ./deploy_all \
  && cd / \
  && rm -rf mr4c \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/scylladb/seastar \
  && cd seastar \
  && ./install-dependencies.sh \
  && ./configure.py --mode=release --prefix=${INSTALL_PREFIX} --without-tests \
  && cmake --build build/release  --parallel \
  && cmake --install build/release --prefix "${INSTALL_PREFIX}" \
  && cd / \
  && rm -rf seastar \
  && pip3 install --break-system-packages scapy pythonping ping3 atheris curio flatbuffers gil thriftpy2 mrjob libp2p bjoern pcapy-ng pycapnp==2.0.0b1 \
                                          eve jwt sanic pyTelegramBotAPI==4.10.0 oct2py~=5.6.0 nats.py opcua asyncua opcua-client proxy-py \
  && pip3 install --break-system-packages --extra-index-url "https://gitlab.com/api/v4/projects/19904939/packages/pypi/simple" pythonfuzz \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/matt-42/lithium.git \
  && cd lithium \
#  && ./vcpkg/bootstrap-vcpkg.sh \
#  && vcpkg/vcpkg install \
#  && cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake . \
#  && cmake --build . --parallel \
#  && cmake --install . --prefix "${INSTALL_PREFIX}/li" \
  && g++ -O3 -DNDEBUG -std=c++17 libraries/symbol/symbol/symbol_generator.cc -o ${INSTALL_PREFIX}/bin/li_symbol_generator \
  && find single_headers -name "*.hh" -exec cp {} ${INSTALL_PREFIX}/include \; \
  && cd / \
  && rm -rf lithium \
#  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/libp2p/cpp-libp2p.git \
#  && cd cpp-libp2p \
#  && cmake -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DTESTING=OFF -DEXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -G "Ninja" -B build . \
#  && cmake --build build --parallel \
#  && cmake --install build --prefix "${INSTALL_PREFIX}" \
#  && cd / \
#  && rm -rf cpp-libp2p \
  && git clone ${GIT_CLONE_PARAMETERS} https://github.com/emweb/wt \
  && cd wt \
  && cmake -B build -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} -DBUILD_TESTS=OFF -DINSTALL_EXAMPLES=ON . \
  && cmake --build build \
  && cmake --install build --prefix "${INSTALL_PREFIX}" \
  && cd / \
  && rm -rf wt \
  && apt-get clean \
  && ldconfig

COPY ./entry.sh /
RUN chmod +x /entry.sh
COPY ./bashrc /etc/skel/.bashrc
# COPY ./yggdrasil.conf /etc/yggdrasil.conf

ENTRYPOINT ["/entry.sh"]
CMD ["/bin/bash"]
