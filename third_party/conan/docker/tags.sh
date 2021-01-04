# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Maps conan profiles to docker image tags. The images are expected to be named after the conan profile,
# but the tag can be adjusted here to support versioning of docker containers. This map only lists
# docker images that have versioning support enabled. For all other docker containers the tag `latest`
# should be / will be assumed.
declare -rA docker_image_tag_mapping=( \
)

# Use `readonly DOCKER_IMAGE_TAG="${docker_image_tag_mapping[${CONAN_PROFILE}]-latest}"`
# to obtain the tag and fall back to `latest` if none is given.
