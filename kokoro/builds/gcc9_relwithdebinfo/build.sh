#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Fail on any error.
set -e

readonly DIR="/mnt/github/orbitprofiler"
readonly SCRIPT="${DIR}/kokoro/builds/gcc9_relwithdebinfo/build.sh"
readonly CONAN_PROFILE="gcc9_relwithdebinfo"

if [ "$0" == "$SCRIPT" ]; then
  # We are inside the docker container

  pip3 install conan==1.27.1 conan-package-tools==0.34.0

  echo "Installing conan configuration (profiles, settings, etc.)..."
  ${DIR}/third_party/conan/configs/install.sh || exit $?

  CRASHDUMP_SERVER=""

  # Building Orbit
  mkdir -p "${DIR}/build/"
  cp -v "${DIR}/third_party/conan/lockfiles/linux/${CONAN_PROFILE}/conan.lock" \
        "${DIR}/build/conan.lock"
  sed -i -e "s|crashdump_server=|crashdump_server=$CRASHDUMP_SERVER|" \
            "${DIR}/build/conan.lock"
  conan install -u -pr ${CONAN_PROFILE} -if "${DIR}/build/" \
          --build outdated \
          -o crashdump_server="$CRASHDUMP_SERVER" \
          --lockfile="${DIR}/build/conan.lock" \
          "${DIR}"
  conan build -bf "${DIR}/build/" "${DIR}"
  conan package -bf "${DIR}/build/" "${DIR}"

else
  gcloud auth configure-docker --quiet
  docker run --rm --network host -v ${KOKORO_ARTIFACTS_DIR}:/mnt gcr.io/orbitprofiler/${CONAN_PROFILE}:latest $SCRIPT
fi

