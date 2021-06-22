# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Maps conan profiles to docker image digests.
# Unlike tags digest hashes can't be overwritten and allows us to require
# an exact version of a container.
declare -rA docker_image_digest_mapping=(
  [clang_format]="gcr.io/orbitprofiler/clang_format@sha256:6729c09de588cf10c7385dcd7efb3dfbeae9b1d866eda1f5ac23a4379835d216" \
  [ggp_release]="gcr.io/orbitprofiler/ggp_release@sha256:e52179e3b1c31a06d0ae92a8f750963c2696cf741506290baa340dfa63c437ca" \
  [ggp_relwithdebinfo]="gcr.io/orbitprofiler/ggp_release@sha256:e52179e3b1c31a06d0ae92a8f750963c2696cf741506290baa340dfa63c437ca" \
  [ggp_debug]="gcr.io/orbitprofiler/ggp_debug@sha256:e52179e3b1c31a06d0ae92a8f750963c2696cf741506290baa340dfa63c437ca" \
  [license_headers]="gcr.io/orbitprofiler/license_headers@sha256:f131914386ba4cd91610c89722bfcdf02eba1bf5d47ed49050a629df28173e93" \
)
