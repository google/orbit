# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Maps conan profiles to docker image digests.
# Unlike tags digest hashes can't be overwritten and allows us to require
# an exact version of a container.
declare -rA docker_image_digest_mapping=(
  [clang_format]="gcr.io/orbitprofiler/clang_format@sha256:6729c09de588cf10c7385dcd7efb3dfbeae9b1d866eda1f5ac23a4379835d216" \
  [clang7_release]="gcr.io/orbitprofiler/clang7_release@sha256:bd8f405aad495dc67c0162401cae73996e5fa9a71ea352d53f9928c98cfc87f5" \
  [clang7_relwithdebinfo]="gcr.io/orbitprofiler/clang7_release@sha256:bd8f405aad495dc67c0162401cae73996e5fa9a71ea352d53f9928c98cfc87f5" \
  [clang7_debug]="gcr.io/orbitprofiler/clang7_debug@sha256:bd8f405aad495dc67c0162401cae73996e5fa9a71ea352d53f9928c98cfc87f5" \
  [clang8_release]="gcr.io/orbitprofiler/clang8_release@sha256:f9ec4c1ce0eb7e1a8d77569e36d3c7c230712bf8e215bc77ed25001015351ddb" \
  [clang8_relwithdebinfo]="gcr.io/orbitprofiler/clang8_release@sha256:f9ec4c1ce0eb7e1a8d77569e36d3c7c230712bf8e215bc77ed25001015351ddb" \
  [clang8_debug]="gcr.io/orbitprofiler/clang8_debug@sha256:f9ec4c1ce0eb7e1a8d77569e36d3c7c230712bf8e215bc77ed25001015351ddb" \
  [clang9_release]="gcr.io/orbitprofiler/clang9_release@sha256:1ed8154b371a5503a90cdce9fd437c31ca68f91c7a738fe233ef9f8c5d5132de" \
  [clang9_relwithdebinfo]="gcr.io/orbitprofiler/clang9_release@sha256:1ed8154b371a5503a90cdce9fd437c31ca68f91c7a738fe233ef9f8c5d5132de" \
  [clang9_debug]="gcr.io/orbitprofiler/clang9_debug@sha256:1ed8154b371a5503a90cdce9fd437c31ca68f91c7a738fe233ef9f8c5d5132de" \
  [gcc9_release]="gcr.io/orbitprofiler/gcc9_release@sha256:0a2e07d422122cd9fb4413149ca1e16a69cd6d7dd5989ed7938b221b4be32f45" \
  [gcc9_relwithdebinfo]="gcr.io/orbitprofiler/gcc9_release@sha256:0a2e07d422122cd9fb4413149ca1e16a69cd6d7dd5989ed7938b221b4be32f45" \
  [gcc9_debug]="gcr.io/orbitprofiler/gcc9_debug@sha256:0a2e07d422122cd9fb4413149ca1e16a69cd6d7dd5989ed7938b221b4be32f45" \
  [ggp_release]="gcr.io/orbitprofiler/ggp_release@sha256:e52179e3b1c31a06d0ae92a8f750963c2696cf741506290baa340dfa63c437ca" \
  [ggp_relwithdebinfo]="gcr.io/orbitprofiler/ggp_release@sha256:e52179e3b1c31a06d0ae92a8f750963c2696cf741506290baa340dfa63c437ca" \
  [ggp_debug]="gcr.io/orbitprofiler/ggp_debug@sha256:e52179e3b1c31a06d0ae92a8f750963c2696cf741506290baa340dfa63c437ca" \
  [iwyu]="gcr.io/orbitprofiler/iwyu@sha256:0cfab7678821b0df872595f2fdcb042bf6ad00ad20fba8873e65684cc7783be9" \
  [license_headers]="gcr.io/orbitprofiler/license_headers@sha256:f131914386ba4cd91610c89722bfcdf02eba1bf5d47ed49050a629df28173e93" \
)
