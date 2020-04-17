#!/bin/bash

function conan_profile_exists {
  conan profile show $profile >/dev/null 2>&1
  return $?
}


REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd )"

# Path to script inside the docker container
SCRIPT="/mnt/contrib/conan/scripts/build_and_upload_dependencies.sh"

if [ "$1" ]; then
  if [ $(uname -s) == "Linux" ]; then
    conan config install $REPO_ROOT/contrib/conan/configs/linux || exit $?
  else
    conan config install $REPO_ROOT/contrib/conan/configs/windows || exit $?
  fi
  conan user -r bintray hebecker -p $BINTRAY_API_KEY || exit $?

  for profile in $@; do
    conan_profile_exists "$profile" || exit 128
    echo "Checking profile $profile..."

    PACKAGES=$(conan info -pr $profile $REPO_ROOT -j 2>/dev/null \
               | grep build_id \
               | jq '.[] | select(.is_ref) | select(.binary != "Download" and .binary != "Cache") | .reference + ":" + .id' \
               | grep -v 'llvm/' \
               | grep -v 'ggp_sdk/' \
               | tr -d '"')

    if [ $(echo -n "$PACKAGES" | wc -c) -eq 0 ]; then
      echo -n "No binary packages are missing or outdated for profile $profile."
      echo " Skipping this configuration."
    else
      echo -e "The following binary packages need to be uploaded:\n$PACKAGES"

      $REPO_ROOT/build.sh $profile || exit $?

      echo "$PACKAGES" | while read package; do
        conan upload -r bintray -c $package
      done
    fi
  done
else
  if [ $(uname -s) == "Linux" ]; then
    PROFILES=( {clang{7,8,9},gcc{8,9},ggp}_{release,relwithdebinfo,debug} )

    for profile in ${PROFILES[@]}; do
      docker run --rm -it -v $REPO_ROOT:/mnt -e BINTRAY_API_KEY \
             gcr.io/orbitprofiler/$profile:latest $SCRIPT $profile || exit $?
    done
  else # Windows
    PROFILES=( msvc{2017,2019}_{release,relwithdebinfo,debug}_{,x86} )

    for profile in ${PROFILES[@]}; do
      $0 $profile || exit $?
    done
  fi
fi
