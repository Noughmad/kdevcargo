FROM ubuntu:artful

RUN apt-get update && apt-get install -y \
    build-essential \
    kdevplatform-dev \
    cmake \
    extra-cmake-modules \
    gettext \
    libkf5itemmodels-dev

COPY . /usr/src/kdevcargo
WORKDIR /usr/src/kdevcargo

RUN mkdir build && cd build && cmake .. && make
