FROM artiomn/nb-build-image

RUN apt-get update \
  && apt-get -y --no-install-recommends --show-progress install \
     clang-tidy \
     clazy \
     isoquery \
     qmlscene qt5-qmltooling-plugins qtcreator-doc \
  	 qtdeclarative5-dev-tools qttools5-dev-tools qttranslations5-l10n \
     qtxmlpatterns5-dev-tools xterm \
     qtcreator \
     valgrind \
     xfonts-cyrillic \
  && apt-get clean

CMD ["/usr/bin/qtcreator"]
