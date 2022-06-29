FROM debian:10 AS builder

RUN apt-get update \
    && apt-get install --no-install-recommends -y unzip libpcap-dev build-essential git cmake libpcap0.8

WORKDIR /src

# RUN git clone --recursive --depth 1 https://github.com/lwip-tcpip/lwip
COPY lwip .
COPY . lwip-echo

RUN \
    cmake -B build lwip-echo \
    && cmake --build build --parallel \
    && mkdir /app \
    && cp build/lwip-runner /app

WORKDIR /app
CMD /app/lwip-runner
