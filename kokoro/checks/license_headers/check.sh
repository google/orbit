#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Fail on any error.
readonly DIR="/mnt/github/orbitprofiler"
readonly SCRIPT="${DIR}/kokoro/checks/license_headers/check.sh"

if [ "$0" == "$SCRIPT" ]; then
  # We are inside the docker container

  cd "$DIR"
  REFERENCE="origin/master"
  MERGE_BASE="$(git merge-base $REFERENCE HEAD)" # Merge base is the commit on master this PR was branched from.
  LICENSE_HEADER_MISSED=""

  echo -e "The following files are missing a license/copyright header:\n"
  while read file; do
    if [ ! -f "$file" ]; then
      continue
    fi

    if ! grep -E 'Copyright \(c\) [0-9]{4} The Orbit Authors\. All rights reserved\.' "$file" > /dev/null; then
      LICENSE_HEADER_MISSED="yes"
      echo "$file"
    fi

  done <<< $(git diff -U0 --no-color --relative --name-only $MERGE_BASE \
  | grep -v third_party/ \
  | grep -v /build \
  | egrep '\.cpp$|\.h$|CMakeLists\.txt$')

  if [ -n "$LICENSE_HEADER_MISSED" ]; then
    exit 1
  fi

else
  gcloud auth configure-docker --quiet
  docker run --rm --network host -v ${KOKORO_ARTIFACTS_DIR}:/mnt gcr.io/orbitprofiler/license_headers:latest $SCRIPT
fi
