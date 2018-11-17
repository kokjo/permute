FROM debian:stretch
RUN dpkg --add-architecture i386
RUN apt-get update && apt-get install -qy libcapstone3:i386 build-essential libc6-dev-i386 libcapstone-dev:i386
WORKDIR /code
COPY . .
RUN make clean && make
