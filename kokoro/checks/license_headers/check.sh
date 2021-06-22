#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Fail on any error.
set -euo pipefail

readonly REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd )"
readonly SCRIPT="/mnt/kokoro/checks/license_headers/check.sh"

if [ "$0" == "$SCRIPT" ]; then
  # We are inside the docker container

  cd /mnt
  # Use origin/main as reference branch, if not specified by kokoro
  REFERENCE="origin/${KOKORO_GITHUB_PULL_REQUEST_TARGET_BRANCH:-main}"
  MERGE_BASE="$(git merge-base $REFERENCE HEAD)" # Merge base is the commit on main this PR was branched from.
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

  done <<< $(git diff -U0 --no-color --relative --name-only --diff-filter=r $MERGE_BASE \
  | grep -v third_party/ \
  | grep -v /build \
  | egrep '\.cpp$|\.h$|CMakeLists\.txt$|\.js$|\.proto$|\.py$')

  if [ -n "$LICENSE_HEADER_MISSED" ]; then
    exit 1
  fi

else
  readonly CONAN_PROFILE="license_headers"

  # Obtain current docker image tags and digests
  source "$(cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd)/third_party/conan/docker/tags.sh"
  source "$(cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd)/third_party/conan/docker/digests.sh"

  # Use a specific tag from the mapping. If none is specified, fall back to `latest`.
  readonly DOCKER_IMAGE_TAG="${docker_image_tag_mapping[${CONAN_PROFILE}]-latest}"

  CONTAINER="gcr.io/orbitprofiler/${CONAN_PROFILE}:${DOCKER_IMAGE_TAG}"

  if [ ${docker_image_digest_mapping[${CONAN_PROFILE}]+abc} ]; then
    echo "Found a docker image digest. Using that to pin the container to an exact version."
    CONTAINER="${docker_image_digest_mapping[${CONAN_PROFILE}]}"
  fi

  gcloud auth configure-docker --quiet
  docker run --rm --network host -v ${REPO_ROOT}:/mnt -e KOKORO_GITHUB_PULL_REQUEST_TARGET_BRANCH \
    ${CONTAINER} $SCRIPT
fi
