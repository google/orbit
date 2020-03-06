#!/bin/bash

function create_conan_profile {
  local profile="$1"
  conan profile new --detect $profile
  conan profile update settings.compiler.libcxx=libstdc++11 $profile

  if [ "$profile" == "default_debug" ]; then
    conan profile update settings.build_type=Debug $profile
  elif [ "$profile" == "default_relwithdebinfo" ]; then
    conan profile update settings.build_type=RelWithDebInfo $profile
  else
    conan profile update settings.build_type=Release $profile
  fi

  sed -i -e 's|\[build_requires\]|[build_requires]\ncmake/3.16.4@|' $HOME/.conan/profiles/$profile
}

function conan_profile_exists {
  conan profile show $profile >/dev/null 2>&1
  return $?
}


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd )"

if [ "$0" == "/mnt/contrib/conan/scripts/build_and_upload_dependencies.sh" -o $(uname -s) != "Linux" ]; then

  conan config install $REPO_ROOT/contrib/conan/config || exit $?
  conan user -r bintray hebecker -p $BINTRAY_API_KEY || exit $?

  if [ $(uname -s) == "Linux" ]; then
    sudo apt-get update -q -q || exit $?
    sudo apt-get install -q -q -y libglu1-mesa-dev mesa-common-dev libxmu-dev libxi-dev qt5-default jq || exit $?
    PROFILES=( default_{release,debug,relwithdebinfo} )
  else # Windows
    PROFILES=( msvc{2017,2019}_{debug,release,relwithdebinfo}_{x64,x86} )
  fi

  for profile in ${PROFILES[@]}; do
    conan_profile_exists "$profile" || create_conan_profile "$profile"

    PACKAGES=$(conan info -pr $profile $REPO_ROOT -j | grep build_id | jq '.[] | select(.is_ref) | select(.binary != "Download" and .binary != "Cache") | .reference + ":" + .id' | grep -v 'llvm/' | grep -v 'ggp_sdk/' | tr -d '"')
    if [ $(echo -n "$PACKAGES" | wc -l) -eq 0 ]; then
      echo "No binary packages are missing or outdated. Skipping this configuration."
    else
      echo -e "The following binary packages need to be uploaded:\n$PACKAGES"

      $REPO_ROOT/build.sh $profile || exit $?

      echo "$PACKAGES" | while read package; do
        conan upload -r bintray -c $package
      done
    fi
  done
else
  CONTAINERS=( gcc8 gcc9 clang7 clang8 clang9 )

  for container in ${CONTAINERS[@]}; do
    docker run --rm -it -v $REPO_ROOT:/mnt -e BINTRAY_API_KEY conanio/$container:latest /mnt/contrib/conan/scripts/build_and_upload_dependencies.sh
  done
fi
