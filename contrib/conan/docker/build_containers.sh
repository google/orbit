#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

declare -A profile_to_dockerfile=( \
  ["clang7_debug"]="clang7_opengl_qt" \
  ["clang7_release"]="clang7_opengl_qt" \
  ["clang7_relwithdebinfo"]="clang7_opengl_qt" \
  ["clang8_debug"]="clang8_opengl_qt" \
  ["clang8_release"]="clang8_opengl_qt" \
  ["clang8_relwithdebinfo"]="clang8_opengl_qt" \
  ["clang9_debug"]="clang9_opengl_qt" \
  ["clang9_release"]="clang9_opengl_qt" \
  ["clang9_relwithdebinfo"]="clang9_opengl_qt" \
  ["ggp_debug"]="clang7_gpg" \
  ["ggp_release"]="clang7_gpg" \
  ["ggp_relwithdebinfo"]="clang7_gpg" \
  ["gcc8_debug"]="gcc8" \
  ["gcc8_release"]="gcc8" \
  ["gcc8_relwithdebinfo"]="gcc8" \
  ["gcc9_debug"]="gcc9" \
  ["gcc9_release"]="gcc9" \
  ["gcc9_relwithdebinfo"]="gcc9" \
  ["msvc2017_release"]="msvc2017" \
  ["msvc2017_relwithdebinfo"]="msvc2017" \
  ["msvc2017_debug"]="msvc2017" \
  ["msvc2017_release_x86"]="msvc2017" \
  ["msvc2017_relwithdebinfo_x86"]="msvc2017" \
  ["msvc2017_debug_x86"]="msvc2017" \
  ["msvc2019_release"]="msvc2019" \
  ["msvc2019_relwithdebinfo"]="msvc2019" \
  ["msvc2019_debug"]="msvc2019" \
  ["msvc2019_release_x86"]="msvc2019" \
  ["msvc2019_relwithdebinfo_x86"]="msvc2019" \
  ["msvc2019_debug_x86"]="msvc2019" )

if [ "$(uname -s)" == "Linux" ]; then
  for profile in {clang{7,8,9},gcc{8,9},ggp}_{release,relwithdebinfo,debug}; do
    docker build "${DIR}" -f "${DIR}/Dockerfile.${profile_to_dockerfile[$profile]}" -t gcr.io/orbitprofiler/$profile:latest || exit $?
  done
else
  for profile in msvc{2017,2019}_{release,relwithdebinfo,debug}; do
    docker build "${DIR}" -f "${DIR}/Dockerfile.${profile_to_dockerfile[$profile]}" -t gcr.io/orbitprofiler/$profile:latest || exit $?
  done
fi
