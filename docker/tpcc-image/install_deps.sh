#!/bin/sh

set -e -x

apt-get update

apt-get install -y \
  ssh \
	make \
	cmake \
	ninja-build \
	git \
	clang-8 \
	clang-format-8 \
	clang-tidy-8 \
	python3 \
	python3-pip \
	ca-certificates \
	openssh-server \
	rsync \
	lldb-8 \
	vim \
  gdb \
  libboost-all-dev \
  wget \
	autoconf

# Install asio lib.
mkdir asio-lib
cd asio-lib
wget https://launchpad.net/ubuntu/+archive/primary/+sourcefiles/asio/1:1.12.2-1/asio_1.12.2.orig.tar.gz
tar -zxvf asio_1.12.2.orig.tar.gz
cd ./asio-1.12.2/
autoreconf -i
./configure --without-boost
make
make install
cd /
rm -rf asio-lib

pip3 install \
	click \
	gitpython \
	python-gitlab \
	termcolor \
	virtualenv
