#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

CONAN_VERSION_REQUIRED="1.53.0"

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
                             libxmu-dev libxi-dev libopengl-dev qtbase5-dev \
                             libxxf86vm-dev python3-pip libboost-dev )

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
  if which dpkg-query >/dev/null 2>&1; then
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
Here are Debian package names: ${REQUIRED_PACKAGES[@]}.

On other Linux distributions the package names might be similar but not exactly
the same!
EOF
  fi # which dpkg-query
fi # IGNORE_SYS_REQ

function check_conan_version_sufficient() {
  local VERSION=$1
  local REQUIRED=$2

  local VERSION_MAJOR="$(echo "$VERSION" | cut -d'.' -f1)"
  local REQUIRED_MAJOR="$(echo "$REQUIRED" | cut -d'.' -f1)"
  if [ "$REQUIRED_MAJOR" -gt "$VERSION_MAJOR" ]; then
    return 1
  fi
  if [ "$REQUIRED_MAJOR" -lt "$VERSION_MAJOR" ]; then
    return 0
  fi

  local VERSION_MINOR="$(echo "$VERSION" | cut -d'.' -f2)"
  local REQUIRED_MINOR="$(echo "$REQUIRED" | cut -d'.' -f2)"
  if [ "$REQUIRED_MINOR" -gt "$VERSION_MINOR" ]; then
    return 1
  fi
  if [ "$REQUIRED_MINOR" -lt "$VERSION_MINOR" ]; then
    return 0
  fi

  local VERSION_PATCH="$(echo "$VERSION" | cut -d'.' -f3)"
  local REQUIRED_PATCH="$(echo "$REQUIRED" | cut -d'.' -f3)"
  if [ "$REQUIRED_PATCH" -gt "$VERSION_PATCH" ]; then
    return 1
  fi
  return 0
}

echo "Checking if conan is available..."
which conan >/dev/null
if [ $? -ne 0 ]; then
  echo "Couldn't find conan. Trying to install via pip..."
  pip3 install --user conan=="$CONAN_VERSION_REQUIRED" || exit $?

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
  
  if ! check_conan_version_sufficient "$CONAN_VERSION" "$CONAN_VERSION_REQUIRED"; then
    echo "Your conan version $CONAN_VERSION is too old. I will try to update..."
    pip3 install --upgrade --user conan=="$CONAN_VERSION_REQUIRED"
    if [ $? -ne 0 ]; then
      echo "The upgrade of your conan installation failed. Probably because conan was not installed by this script."
      echo "Please manually update conan to at least version $CONAN_VERSION_REQUIRED."
      exit 2
    fi
    echo "Conan updated finished."
  else
    echo "Conan's version $CONAN_VERSION fulfills the requirements. Continuing..."
  fi
fi

echo "Installing conan configuration (profiles, settings, etc.)..."
$DIR/third_party/conan/configs/install.sh $CONFIG_INSTALL_OPTIONS || exit $?

if [[ $DONT_COMPILE != "yes" ]]; then
  if [ -n "$1" ] ; then
    exec $DIR/build.sh "$@"
  else
    exec $DIR/build.sh
  fi
fi

PRE_COMMIT_HOOK_FILE=.git/hooks/pre-commit

function install_clang_format_pre_commit_hook {
  if [ ! -f $PRE_COMMIT_HOOK_FILE ] ; then
    touch $PRE_COMMIT_HOOK_FILE
    chmod +x $PRE_COMMIT_HOOK_FILE
  fi
  echo "source \"contrib/hooks/clang-format-pre-commit.sh\"" >> $PRE_COMMIT_HOOK_FILE
}

if [ -d .git ] ; then
  if [ ! -f $PRE_COMMIT_HOOK_FILE ] || ! grep -q "contrib/hooks/clang-format-pre-commit.sh" .git/hooks/pre-commit ; then
    while true; do
      read -p "Do you want to install the clang-format pre-commit hook [y/n]? `echo $'\n> '`" yn
      case $yn in
        [Yy]* ) install_clang_format_pre_commit_hook; break;;
        [Nn]* ) break;;
      esac
    done
  fi
fi
