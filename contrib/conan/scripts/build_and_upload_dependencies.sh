#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd )"

if [ "$0" == "/mnt/contrib/conan/scripts/build_and_upload_dependencies.sh" ]; then

  conan config install $REPO_ROOT/contrib/conan/config || exit $?
  conan user -r bintray hebecker -p $BINTRAY_API_KEY || exit $?

  sudo apt-get update || exit $?
  sudo apt-get install -y libglu1-mesa-dev mesa-common-dev libxmu-dev libxi-dev qt5-default jq || exit $?

  $REPO_ROOT/build.sh default_release default_debug || exit $?
  $DIR/upload.sh default_release default_debug

else
  CONTAINERS=( gcc8 gcc9 clang7 clang8 clang9 )

  for container in ${CONTAINERS[@]}; do
    docker run --rm -it -v $REPO_ROOT:/mnt -e BINTRAY_API_KEY conanio/$container:latest /mnt/contrib/conan/scripts/build_and_upload_dependencies.sh
  done
fi
