# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Maps conan profiles to docker image digests.
# Unlike tags digest hashes can't be overwritten and allows us to require
# an exact version of a container.
declare -rA docker_image_digest_mapping=(
  [clang_format]="gcr.io/orbitprofiler/clang_format@sha256:6729c09de588cf10c7385dcd7efb3dfbeae9b1d866eda1f5ac23a4379835d216" \
  [clang7_release]="gcr.io/orbitprofiler/clang7_release@sha256:5ea84ecedcbae6a69551032fc1f89d69d6282e27e925f9edb2c7451e094d251c" \
  [clang7_relwithdebinfo]="gcr.io/orbitprofiler/clang7_release@sha256:5ea84ecedcbae6a69551032fc1f89d69d6282e27e925f9edb2c7451e094d251c" \
  [clang7_debug]="gcr.io/orbitprofiler/clang7_debug@sha256:5ea84ecedcbae6a69551032fc1f89d69d6282e27e925f9edb2c7451e094d251c" \
  [clang8_release]="gcr.io/orbitprofiler/clang8_release@sha256:9b17376897a8185bd793c0ee037d2851cb3eca2602021688d1154413fcbe7925" \
  [clang8_relwithdebinfo]="gcr.io/orbitprofiler/clang8_release@sha256:9b17376897a8185bd793c0ee037d2851cb3eca2602021688d1154413fcbe7925" \
  [clang8_debug]="gcr.io/orbitprofiler/clang8_debug@sha256:9b17376897a8185bd793c0ee037d2851cb3eca2602021688d1154413fcbe7925" \
  [clang9_release]="gcr.io/orbitprofiler/clang9_release@sha256:02f1a69425bcf6039e309da82c1bcf4aa2aabbe1d45e4b481579b08b76f2a29d" \
  [clang9_relwithdebinfo]="gcr.io/orbitprofiler/clang9_release@sha256:02f1a69425bcf6039e309da82c1bcf4aa2aabbe1d45e4b481579b08b76f2a29d" \
  [clang9_debug]="gcr.io/orbitprofiler/clang9_debug@sha256:02f1a69425bcf6039e309da82c1bcf4aa2aabbe1d45e4b481579b08b76f2a29d" \
  [coverage_clang9]="gcr.io/orbitprofiler/coverage_clang9@sha256:4117216d27fe353879d36b5f3cbeadf6a4bb2f3ef4c0508b25ee4451f4b9e165" \
  [gcc9_release]="gcr.io/orbitprofiler/gcc9_release@sha256:35e1f661e5a647a187ee66b74e1f79044eba534239cccffb8631a7251ac079da" \
  [gcc9_relwithdebinfo]="gcr.io/orbitprofiler/gcc9_release@sha256:35e1f661e5a647a187ee66b74e1f79044eba534239cccffb8631a7251ac079da" \
  [gcc9_debug]="gcr.io/orbitprofiler/gcc9_debug@sha256:35e1f661e5a647a187ee66b74e1f79044eba534239cccffb8631a7251ac079da" \
  [gcc10_release]="gcr.io/orbitprofiler/gcc10_release@sha256:3479c24eaa2512618ecba1872743495583d531af0b5cbd7acf52bc73da54fbeb" \
  [gcc10_relwithdebinfo]="gcr.io/orbitprofiler/gcc10_release@sha256:3479c24eaa2512618ecba1872743495583d531af0b5cbd7acf52bc73da54fbeb" \
  [gcc10_debug]="gcr.io/orbitprofiler/gcc10_debug@sha256:3479c24eaa2512618ecba1872743495583d531af0b5cbd7acf52bc73da54fbeb" \
  [ggp_release]="gcr.io/orbitprofiler/ggp_debug@sha256:e5e4cd26fdf4777f84d18c85b057d06d47faaf5c0cd1f8f76399f4185ec86eb6" \
  [ggp_relwithdebinfo]="gcr.io/orbitprofiler/ggp_debug@sha256:e5e4cd26fdf4777f84d18c85b057d06d47faaf5c0cd1f8f76399f4185ec86eb6" \
  [ggp_debug]="gcr.io/orbitprofiler/ggp_debug@sha256:e5e4cd26fdf4777f84d18c85b057d06d47faaf5c0cd1f8f76399f4185ec86eb6" \
  [iwyu]="gcr.io/orbitprofiler/iwyu@sha256:0cfab7678821b0df872595f2fdcb042bf6ad00ad20fba8873e65684cc7783be9" \
  [license_headers]="gcr.io/orbitprofiler/license_headers@sha256:f131914386ba4cd91610c89722bfcdf02eba1bf5d47ed49050a629df28173e93" \
  [msvc2019_release]="gcr.io/orbitprofiler/msvc2019_release@sha256:943e8ad0fb5ca00f9e4b4edf3e58e9d2330f3d7148106801a6a7b80d19a00c76" \
  [msvc2019_relwithdebinfo]="gcr.io/orbitprofiler/msvc2019_release@sha256:943e8ad0fb5ca00f9e4b4edf3e58e9d2330f3d7148106801a6a7b80d19a00c76" \
  [msvc2019_debug]="gcr.io/orbitprofiler/msvc2019_debug@sha256:943e8ad0fb5ca00f9e4b4edf3e58e9d2330f3d7148106801a6a7b80d19a00c76" \
)
