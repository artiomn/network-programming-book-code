FROM debian:11.6

RUN apt update
RUN apt install octave python3 python3-pip git -y

WORKDIR /opt/octabot
ADD * /opt/octabot/
RUN pip3 install -r requirements.txt

RUN useradd -ms /bin/bash botuser
USER botuser

CMD ["sh","/opt/octabot/start.sh"]
