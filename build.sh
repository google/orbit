#!/bin/bash

default_profiles=( default_release default_debug )

if [ "$#" -eq 0 ]; then
  profiles=( "${default_profiles[@]}" )
else
  profiles=()
  for profile in "$@"; do profiles+=( "$profile" ); done
fi

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

function create_conan_profile {
  local profile="$1"
  conan profile new --detect $profile
  conan profile update settings.compiler.libcxx=libstdc++11 $profile

  if [ "$profile" == "default_debug" ]; then
    conan profile update settings.build_type=Debug $profile
  else
    conan profile update settings.build_type=Release $profile
  fi

  sed -i -e 's|\[build_requires\]|[build_requires]\ncmake/3.16.4@|' $HOME/.conan/profiles/$profile
}

function conan_profile_exists {
  conan profile show $profile >/dev/null 2>&1
  return $?
}


for profile in ${profiles[@]}; do
  if [ "$profile" == "default_release" -o "$profile" == "default_debug" ]; then
    conan_profile_exists "$profile" || create_conan_profile "$profile"
  fi

  conan install -u -pr $profile -if build_$profile/ --build outdated $DIR || exit $?
  conan build -bf build_$profile/ $DIR || exit $?
  conan package -bf build_$profile/ $DIR || exit $?
done
