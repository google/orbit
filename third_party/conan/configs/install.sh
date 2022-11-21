#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

OPTIONS=$(getopt -o "" -l "assume-linux,assume-windows" -- "$@")
eval set -- "$OPTIONS"

if [ "$(uname -s)" == "Linux" ]; then
  OS="linux"
else
  OS="windows"
fi

while true; do
  case "$1" in
    --assume-linux) OS="linux"; shift;;
    --assume-windows) OS="windows"; shift;;
    --) shift; break;;
  esac
done

# Installs conan config (build settings, public remotes, conan profiles, conan options). Have a look in \third_party\conan\configs\windows for more information.
conan config install "$DIR/$OS" || exit $?

