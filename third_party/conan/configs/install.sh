#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

function conan_disable_public_remotes {
  conan remote disable bintray || return $?
  conan remote disable conan-center || return $?
  conan remote disable bincrafters || return $?
  return 0
}

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Installs conan config (build settings, public remotes, conan profiles, conan options). Have a look in \third_party\conan\configs\windows for more information.
if [ "$(uname -s)" == "Linux" ]; then
  conan config install "$DIR/linux" || exit $?
else
  conan config install "$DIR/windows" || exit $?
fi

if [ -n "$ORBIT_OVERRIDE_ARTIFACTORY_URL" ]; then
  echo "Artifactory override detected. Adjusting remotes..."
  conan remote add -i 0 -f artifactory "$ORBIT_OVERRIDE_ARTIFACTORY_URL" || exit $?
  conan_disable_public_remotes || exit $?
else
  curl -s http://artifactory.internal/ >/dev/null 2>&1
  
  if [ $? -eq 0 ]; then
    echo "CI machine detected. Adjusting remotes..."
    conan remote add -i 0 -f artifactory "http://artifactory.internal/artifactory/api/conan/conan" || exit $?
    conan_disable_public_remotes || exit $?
  else
    LOCATION="$(curl -sI http://orbit-artifactory/ 2>/dev/null)"

    if [ $? -eq 0 ]; then
      echo "Internal machine detected. Adjusting remotes..."
      LOCATION="$(echo "$LOCATION" | grep -e "^Location:" | cut -d ' ' -f 2 | cut -d '/' -f 3)"

      conan remote add -i 0 -f artifactory "http://$LOCATION/artifactory/api/conan/conan" || exit $?
      conan config set proxies.http="$LOCATION=http://127.0.0.2:999" || exit $?
      conan_disable_public_remotes || exit $?
    else
      echo "Using public remotes for conan."
    fi
  fi
fi
