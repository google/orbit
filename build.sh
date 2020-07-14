#!/bin/bash
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


default_profiles=( default_relwithdebinfo )

if [ "$#" -eq 0 ]; then
  profiles=( "${default_profiles[@]}" )
else
  profiles=()
  for profile in "$@"; do profiles+=( "$profile" ); done
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

function create_conan_profile {
  readonly profile="$1"
  if ! conan profile show default >/dev/null; then
    conan profile new --detect default || exit $?
  fi

  readonly compiler="$(conan profile show default | grep compiler= | cut -d= -f2 | sed -e 's/Visual Studio/msvc/')"
  readonly compiler_version="$(conan profile show default | grep compiler.version= | cut -d= -f2 | sed -e 's/^15$/2017/' | sed -e 's/^16$/2019/')"
  readonly conan_dir=${CONAN_USER_HOME:-~}/.conan
  readonly build_type="${profile#default_}"
  readonly profile_path="$conan_dir/profiles/$profile"

  echo -e "include(${compiler}${compiler_version}_${build_type})\n" > $profile_path
  echo -e "[settings]\n[options]" >> $profile_path
  echo -e "[build_requires]\n[env]" >> $profile_path

  if [ -n "$CC" ]; then
    echo "CC=$CC" >> $profile_path
  fi

  if [ -n "$CXX" ]; then
    echo "CXX=$CXX" >> $profile_path
  fi
}

function conan_profile_exists {
  conan profile show $profile >/dev/null 2>&1
  return $?
}


for profile in ${profiles[@]}; do
  if [ "$profile" == "default_release" -o "$profile" == "default_debug" -o "$profile" == "default_relwithdebinfo" ]; then
    conan_profile_exists "$profile" || create_conan_profile "$profile"
  fi

  conan install -u -pr $profile -if build_$profile/ --build outdated $DIR || exit $?
  conan build -bf build_$profile/ $DIR || exit $?
done
