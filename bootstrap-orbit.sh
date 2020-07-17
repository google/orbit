#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

OPTIONS=$(getopt -o "" -l "force-public-remotes,assume-linux,assume-windows,ignore-system-requirements,dont-compile" -- "$@")
eval set -- "$OPTIONS"

CONFIG_INSTALL_OPTIONS=""
IGNORE_SYS_REQUIREMENTS=""
DONT_COMPILE=""

while true; do
  case "$1" in
    --force-public-remotes|--assume-linux|--assume-windows)
      CONFIG_INSTALL_OPTIONS="${CONFIG_INSTALL_OPTIONS} $1"; shift;;
    --ignore-system-requirements) IGNORE_SYS_REQUIREMENTS="yes"; shift;;
    --dont-compile) DONT_COMPILE="yes"; shift;;
    --) shift; break;;
  esac
done

readonly REQUIRED_PACKAGES=( build-essential libglu1-mesa-dev mesa-common-dev \
                             libxmu-dev libxi-dev libopengl0 qt5-default \
                             qtwebengine5-dev libqt5webchannel5-dev \
                             libqt5websockets5-dev libxxf86vm-dev python3-pip )

function add_ubuntu_universe_repo {
  sudo add-apt-repository universe
  if [ $? -ne 0 ]; then
    sudo apt-get install -y software-properties-common
    sudo add-apt-repository universe
  fi
}

function install_required_packages {
  sudo apt-get update || exit $?
  sudo apt-get install -y ${REQUIRED_PACKAGES[@]} || exit $?
}


if [[ $IGNORE_SYS_REQUIREMENTS != "yes" ]]; then
  if which dpkg-query >/dev/null; then
    readonly installed="$(dpkg-query --show -f'${Package}\n')"
    PACKAGES_MISSING="no"
    for package in ${REQUIRED_PACKAGES[@]}; do
      if ! egrep "^${package}$" <<<"$installed" > /dev/null; then
        PACKAGES_MISSING="yes"
        break
      fi
    done

    if [[ $PACKAGES_MISSING == "yes" ]]; then
      echo "Installing required system dependencies..."

      # That only works on Ubuntu!
      if [[ "$(lsb_release -si)" == "Ubuntu" ]]; then
        add_ubuntu_universe_repo
      fi

      install_required_packages
    fi
  else
    cat <<EOF
We detected you're not on a debian-based system. Orbit requires some system
packages to be installed. Please make sure that you have those installed.
Here are Debian package names: $PACKAGE[@].

On other Linux distributions the package names might be similar but not exactly
the same!
EOF
  fi # which dpkg-query
fi # IGNORE_SYS_REQ

echo "Checking if conan is available..."
which conan >/dev/null
if [ $? -ne 0 ]; then
  echo "Couldn't find conan. Trying to install via pip..."
  pip3 install --user conan==1.27.1 || exit $?

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
  echo "Found conan. Checking version..."
  CONAN_VERSION="$(conan --version | cut -d' ' -f3)"
  CONAN_VERSION_MAJOR="$(echo "$CONAN_VERSION" | cut -d'.' -f1)"
  CONAN_VERSION_MINOR="$(echo "$CONAN_VERSION" | cut -d'.' -f2)"

  CONAN_VERSION_MAJOR_REQUIRED=1
  CONAN_VERSION_MINOR_MIN=27

  if [ "$CONAN_VERSION_MAJOR" -eq $CONAN_VERSION_MAJOR_REQUIRED -a "$CONAN_VERSION_MINOR" -lt $CONAN_VERSION_MINOR_MIN ]; then
    echo "Your conan version $CONAN_VERSION is too old. I will try to update..."
    pip3 install --upgrade --user conan
    if [ $? -ne 0 ]; then
      echo "The upgrade of your conan installation failed. Probably because conan was not installed by this script."
      echo "Please manually update conan to at least version $CONAN_VERSION_MAJOR_REQUIRED.$CONAN_VERSION_MINOR_MIN."
      exit 2
    fi
    echo "Conan updated finished."
  else
    echo "Conan's version fulfills the requirements. Continuing..."
  fi
fi

echo "Installing conan configuration (profiles, settings, etc.)..."
$DIR/third_party/conan/configs/install.sh $CONFIG_INSTALL_OPTIONS || exit $?

if [[ $DONT_COMPILE != "yes" ]]; then
  if [ -n "$1" ] ; then
    exec $DIR/build.sh "$@"
  else
    conan remote list | grep -v 'Disabled:' | grep -e '^artifactory:' > /dev/null 2>&1
    if [ $? -eq 0 ]; then
      exec $DIR/build.sh default_relwithdebinfo ggp_relwithdebinfo
    else
      exec $DIR/build.sh
    fi
  fi
fi

