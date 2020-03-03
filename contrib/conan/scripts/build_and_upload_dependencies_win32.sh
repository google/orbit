#!/bin/bash
# Meant to be called from Git-Bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd )"

conan config install $REPO_ROOT/contrib/conan/config || exit $?
conan user -r bintray hebecker -p $BINTRAY_API_KEY || exit $?

PROFILES=( msvc2017_debug_x64 msvc2017_debug_x86 msvc2017_release_x64 msvc2017_release_x86 msvc2019_debug_x64 msvc2019_debug_x86 msvc2019_release_x64 msvc2019_release_x86 )

for profile in ${PROFILES[@]}; do
  conan profile show $profile >/dev/null
  if [ $? -ne 0 ]; then
    echo "Profile $profile not found!"
    exit 1
  fi
done

for profile in ${PROFILES[@]}; do
  echo "Building orbit (and dependencies if necessary) for profile $profile..."
  $REPO_ROOT/build.sh $profile || exit $?
  $DIR/upload.sh $profile
done
