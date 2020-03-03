#!/bin/bash


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

which conan >/dev/null
if [ $? -ne 0 ]; then
        sudo pip3 install conan
fi

# Load Submodules (libunwindstack)
(cd $DIR && git submodule update --init --recursive)

if [ $? -ne 0 ]; then
  echo "Orbit: Could not update/initialize all the submodules. Exiting..."
  exit 1
fi

unset GGP_SDK_PATH

conan config install $DIR/contrib/conan/config || exit $?

# Install Stadia GGP SDK
conan search ggp_sdk | grep orbitdeps/stable > /dev/null
if [ $? -ne 0 ]; then
  if [ ! -d $DIR/contrib/conan/recipes/ggp_sdk ]; then
    git clone sso://user/hebecker/conan-ggp_sdk $DIR/contrib/conan/recipes/ggp_sdk || exit $?
  fi
  conan export $DIR/contrib/conan/recipes/ggp_sdk orbitdeps/stable || exit $?
else
  echo "ggp_sdk seems to be installed already. Skipping installation step..."
fi

$DIR/build.sh ggp_release ggp_debug
