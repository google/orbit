#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo "Loading submodules (libunwindstack, lzma)..."
(cd $DIR && git submodule update --init --recursive)

if [ $? -ne 0 ]; then
  echo "Orbit: Could not update/initialize all the submodules. Exiting..."
  exit 1
fi

echo "Installing conan configuration (profiles, settings, etc.)..."
conan config install $DIR/contrib/conan/config || exit $?

exec $DIR/build.sh default_release

