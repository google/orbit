# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Maps conan profiles to docker image tags. The images are expected to be named after the conan profile,
# but the tag can be adjusted here to support versioning of docker containers. This map only lists
# docker images that have versioning support enabled. For all other docker containers the tag `latest`
# should be / will be assumed.
declare -rA docker_image_tag_mapping=( \
  [iwyu]="2" \
  [coverage_clang9]="1" \
  [clang_format]="20210622T084030.289589958" \
  [clang9_release]="20210622T114306.097973571" \
  [clang9_relwithdebinfo]="20210622T114447.422662718" \
  [clang9_debug]="20210622T114450.448991866" \
  [gcc9_release]="20210622T103629.868794602" \
  [gcc9_relwithdebinfo]="20210622T103809.023418816" \
  [gcc9_debug]="20210622T103812.061618308" \
  [ggp_release]="20210622T105923.736243550" \
  [ggp_relwithdebinfo]="20210622T110005.748389893" \
  [ggp_debug]="20210622T110008.723567043" \
  [license_headers]="20210622T081800.703185280" \
)

# Use `readonly DOCKER_IMAGE_TAG="${docker_image_tag_mapping[${CONAN_PROFILE}]-latest}"`
# to obtain the tag and fall back to `latest` if none is given.
