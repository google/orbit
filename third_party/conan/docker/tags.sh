# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Maps conan profiles to docker image tags. The images are expected to be named after the conan profile,
# but the tag can be adjusted here to support versioning of docker containers. This map only lists
# docker images that have versioning support enabled. For all other docker containers the tag `latest`
# should be / will be assumed.
declare -rA docker_image_tag_mapping=( \
  [coverage_clang9]="1" \
  [clang_format]="20210622T084030.289589958" \
  [clang7_release]="20210622T125530.963597266" \
  [clang7_relwithdebinfo]="20210622T125713.264442242" \
  [clang7_debug]="20210622T125716.404249382" \
  [clang8_release]="20210622T124717.449033412" \
  [clang8_relwithdebinfo]="20210622T124858.908641324" \
  [clang8_debug]="20210622T124901.983409890" \
  [clang9_release]="20210622T114306.097973571" \
  [clang9_relwithdebinfo]="20210622T114447.422662718" \
  [clang9_debug]="20210622T114450.448991866" \
  [gcc9_release]="20210622T103629.868794602" \
  [gcc9_relwithdebinfo]="20210622T103809.023418816" \
  [gcc9_debug]="20210622T103812.061618308" \
  [ggp_release]="20210622T105923.736243550" \
  [ggp_relwithdebinfo]="20210622T110005.748389893" \
  [ggp_debug]="20210622T110008.723567043" \
  [iwyu]="20210623T060051.145174476" \
  [license_headers]="20210622T081800.703185280" \
)

# Use `readonly DOCKER_IMAGE_TAG="${docker_image_tag_mapping[${CONAN_PROFILE}]-latest}"`
# to obtain the tag and fall back to `latest` if none is given.
