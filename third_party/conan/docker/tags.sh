# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Maps conan profiles to docker image tags. The images are expected to be named after the conan profile,
# but the tag can be adjusted here to support versioning of docker containers. This map only lists
# docker images that have versioning support enabled. For all other docker containers the tag `latest`
# should be / will be assumed.
declare -rA docker_image_tag_mapping=( \
  [clang_format]="20210622T084030.289589958" \
  [clang7_release]="20210623T115018.050778297" \
  [clang7_relwithdebinfo]="20210623T115524.844271965" \
  [clang7_debug]="20210623T115534.007364778" \
  [clang8_release]="20210623T115629.862456327" \
  [clang8_relwithdebinfo]="20210623T115813.431147006" \
  [clang8_debug]="20210623T115816.468538889" \
  [clang9_release]="20210623T115201.435218253" \
  [clang9_relwithdebinfo]="20210623T115528.044499634" \
  [clang9_debug]="20210623T115537.014113242" \
  [clang11_release]="20221025T111325.856531974" \
  [clang11_relwithdebinfo]="20221025T111330.643117217" \
  [clang11_debug]="20221025T111335.481687525" \
  [coverage_clang9]="20210811T113809.218464623" \
  [coverage_clang11]="20221025T160349.788715511" \
  [gcc9_release]="20210623T115343.808432117" \
  [gcc9_relwithdebinfo]="20210623T115531.065509473" \
  [gcc9_debug]="20210623T115540.009801343" \
  [gcc10_release]="20221025T141441.105832112" \
  [gcc10_relwithdebinfo]="20221025T141635.001186519" \
  [gcc10_debug]="20221025T141639.834164062" \
  [ggp_release]="20210928T142531.226033147" \
  [ggp_relwithdebinfo]="20210928T142531.072483480" \
  [ggp_debug]="20210928T142429.353834247" \
  [iwyu]="20221025T144000.693492592" \
  [license_headers]="20210622T081800.703185280" \
  [msvc2019_release]="20210928T145527.153573100" \
  [msvc2019_relwithdebinfo]="20210928T153502.151900000" \
  [msvc2019_debug]="20210928T153511.481358200" \
)

# Use `readonly DOCKER_IMAGE_TAG="${docker_image_tag_mapping[${CONAN_PROFILE}]-latest}"`
# to obtain the tag and fall back to `latest` if none is given.
