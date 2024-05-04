FROM debian:11.6

WORKDIR /opt/octabot
ADD * /opt/octabot/

RUN apt update \
  && apt-get --no-install-recommends --show-progress install octave python3 python3-pip git -y \
  && apt-get clean \
  && pip3 install --break-system-packages -r requirements.txt \
  && useradd -ms /bin/bash botuser

USER botuser

CMD ["sh","/opt/octabot/start.sh"]
