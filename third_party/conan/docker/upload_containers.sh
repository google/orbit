#!/bin/bash
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -euo pipefail

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

source "${DIR}/tags.sh"

if [ "$(uname -s)" == "Linux" ]; then
  for profile in {clang{7,8,9},gcc{8,9},ggp}_{release,relwithdebinfo,debug} clang_format license_headers iwyu; do
    tag="${docker_image_tag_mapping[${profile}]-latest}"
    docker push gcr.io/orbitprofiler/$profile:${tag} || exit $?
  done
else
  for profile in msvc{2017,2019}_{release,relwithdebinfo,debug}; do
    tag="${docker_image_tag_mapping[${profile}]-latest}"
    docker push gcr.io/orbitprofiler/$profile:${tag} || exit $?
  done
fi
