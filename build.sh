#!/bin/bash
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -euo pipefail

# We call conan by going through python3 to avoid any PATH problems.
# Sometimes tools installed by PIP are not automatically in the PATH.
if [[ $(uname -s) == "Linux" ]]; then
  CONAN="python3 -m conans.conan"
else
  CONAN="py -3 -m conans.conan"
fi

readonly CONAN

default_profiles=( default_release )

if [ "$#" -eq 0 ]; then
  profiles=( "${default_profiles[@]}" )
else
  profiles=()
  for profile in "$@"; do profiles+=( "$profile" ); done
fi

readonly DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

function create_conan_profile {
  local readonly profile="$1"
  if ! $CONAN profile show default >/dev/null 2>&1; then
    $CONAN profile new --detect default >/dev/null || exit $?
  fi

  local readonly compiler="$($CONAN profile show default | grep compiler= | cut -d= -f2 | sed -e 's/Visual Studio/msvc/')"
  local compiler_version="$($CONAN profile show default | grep compiler.version= | cut -d= -f2)"
  if [[ $compiler == "msvc" ]]; then
    compiler_version="$(echo $compiler_version | sed -e 's/^16$/2019/' | sed -e 's/^17$/2022/')"
  fi
  local readonly compiler_version
  local readonly conan_dir=${CONAN_USER_HOME:-~}/.conan
  local readonly build_type="${profile#default_}"
  local readonly profile_path="$conan_dir/profiles/$profile"

  echo -e "include(${compiler}${compiler_version}_${build_type})\n" > $profile_path
  echo -e "[settings]\n[options]" >> $profile_path
  echo -e "[build_requires]\n[env]" >> $profile_path

  if [[ -v CC ]]; then
    echo "CC=$CC" >> $profile_path
  fi

  if [[ -v CXX ]]; then
    echo "CXX=$CXX" >> $profile_path
  fi
}

function conan_profile_exists {
  $CONAN profile show $1 >/dev/null 2>&1
  return $?
}

# That's the profile that is used for tools that run on the build machine (Nasm, CMake, Ninja, etc.)
readonly build_profile="default_release"
conan_profile_exists "${build_profile}" || create_conan_profile "${build_profile}"

for profile in ${profiles[@]}; do
  if [[ $profile == default_* ]]; then
    conan_profile_exists "$profile" || create_conan_profile "$profile"
  fi

  $CONAN install -pr:b "${build_profile}" -pr:h $profile -if build_$profile/ \
         --build outdated --update "$DIR" || exit $?
  $CONAN build -bf build_$profile/ "$DIR" || exit $?

  if [[ $profile == default_* ]]; then
    echo "The build finished successfully. Start Orbit with ./build_$profile/bin/Orbit!"
  fi
done