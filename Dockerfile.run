FROM debian
RUN dpkg --add-architecture i386
RUN apt-get update && apt-get install -qy libcapstone3:i386
COPY program /program
