#!/bin/sh

if ( which dirname ) && ( which realpath ); then
  BOOTSTRAP_ROOT=$(dirname $(realpath $0))
  cd "${BOOTSTRAP_ROOT}"
  cd ../
fi

git submodule update --init --recursive
client/install.py
