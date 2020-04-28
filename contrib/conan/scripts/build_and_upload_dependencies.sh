#!/bin/bash

function conan_profile_exists {
  conan profile show $profile >/dev/null 2>&1
  return $?
}


REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd )"
REPO_ROOT_WIN="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd -W 2>/dev/null)"

# Path to script inside the docker container
SCRIPT="/mnt/contrib/conan/scripts/build_and_upload_dependencies.sh"

if [ "$1" ]; then
  $REPO_ROOT/contrib/conan/configs/install.sh || exit $?
  conan user -r artifactory $ARTIFACTORY_USERNAME -p $ARTIFACTORY_API_KEY || exit $?

  for profile in $@; do
    conan_profile_exists "$profile" || exit 128
    echo "Checking profile $profile..."

    PACKAGES=$(conan info -pr $profile $REPO_ROOT -j 2>/dev/null \
               | grep build_id \
               | jq '.[] | select(.is_ref) | select(.binary != "Download" and .binary != "Cache" and .binary != "Skip") | .reference + ":" + .id' \
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
        conan upload -r artifactory -c $package
      done
    fi
  done
else
  if [ $(uname -s) == "Linux" ]; then
    PROFILES=( {clang{7,8,9},gcc{8,9},ggp}_{release,relwithdebinfo,debug} )

    if [ -z "${ORBIT_OVERRIDE_ARTIFACTORY_URL=}" ]; then
      echo "Note: SSH port forwarding for artifactory set up?"
      export ORBIT_OVERRIDE_ARTIFACTORY_URL="http://localhost:8080/artifactory/api/conan/conan"
    fi

    for profile in ${PROFILES[@]}; do
      docker run --network host --rm -it -v $REPO_ROOT:/mnt \
             -e ARTIFACTORY_USERNAME -e ARTIFACTORY_API_KEY \
             -e ORBIT_OVERRIDE_ARTIFACTORY_URL \
             gcr.io/orbitprofiler/$profile:latest $SCRIPT $profile || exit $?
    done
  else # Windows
    PROFILES=( msvc{2017,2019}_{release,relwithdebinfo,debug}{,_x86} )

    if [ -z "${ORBIT_OVERRIDE_ARTIFACTORY_URL=}" ]; then
      echo "Note: SSH port forwarding for artifactory set up?"
      PUBLIC_IP="$(ipconfig | grep -A20 "Ethernet adapter Ethernet:" | grep "IPv4 Address" | head -n1 | cut -d ':' -f 2 | tr -d ' \r\n')"
      export ORBIT_OVERRIDE_ARTIFACTORY_URL="http://${PUBLIC_IP}:8080/artifactory/api/conan/conan"
    fi

    for profile in ${PROFILES[@]}; do
      docker run --isolation=process --rm -v $REPO_ROOT_WIN:C:/mnt \
	     -e ARTIFACTORY_USERNAME -e ARTIFACTORY_API_KEY \
	     -e ORBIT_OVERRIDE_ARTIFACTORY_URL \
	     gcr.io/orbitprofiler/$profile:latest "C:/Program Files/Git/bin/bash.exe" \
	     -c "/c$SCRIPT $profile" || exit $?
    done
  fi
fi
