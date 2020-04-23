#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo "Installing required system dependencies..."
sudo add-apt-repository universe
if [ $? -ne 0 ]; then
  sudo apt-get install -y software-properties-common
  sudo add-apt-repository universe
fi

sudo apt-get update
sudo apt-get install -y build-essential
sudo apt-get install -y libglu1-mesa-dev mesa-common-dev libxmu-dev libxi-dev
sudo apt-get install -y libopengl0
sudo apt-get install -y qt5-default
sudo apt-get install -y python3-pip

echo "Checking if conan is available..."
which conan >/dev/null
if [ $? -ne 0 ]; then
  echo "Couldn't find conan. Trying to install via pip..."
  pip3 install --user conan || exit $?

  which conan >/dev/null
  if [ $? -ne 0 ]; then
    echo "Could not find conan in the path, although the installation reported success."
    echo "Probably conan was installed into a directory which is not in the PATH."
    echo "Please ensure conan is reachable via the PATH and call this script again."
    echo "Hint: Probably you have to add $HOME/.local/bin to your path variable."
    echo "      If you use bash, you can call 'export PATH=\$HOME/.local/bin:\$PATH'"
    echo "      to do that. Add this line to your .bashrc file, to make this change"
    echo "      persistent."
    exit 1
  fi
else
  echo "Found conan. Skipping installation..."
fi

echo "Installing conan configuration (profiles, settings, etc.)..."
$DIR/contrib/conan/configs/install.sh || exit $?

exec $DIR/build.sh "$@"

