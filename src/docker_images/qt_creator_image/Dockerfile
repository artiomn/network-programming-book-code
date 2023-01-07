FROM artiomn/nb-build-image

COPY preferences/* /etc/apt/prefernces.d/

RUN \
  echo 'deb http://deb.debian.org/debian bookworm main contrib non-free' >> /etc/apt/sources.list.d/bookworm.list \
  && apt-get update \
  && apt-get -y --no-install-recommends --show-progress install \
     clang-tidy \
     clazy \
     isoquery \
     qtcreator \
     qmlscene qt6-qmltooling-plugins qtcreator-doc \
     xterm \
     valgrind \
     xfonts-cyrillic \
  && apt-get clean

CMD ["/usr/bin/qtcreator"]
