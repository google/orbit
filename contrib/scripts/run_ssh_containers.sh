#!/bin/bash
# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script is a helper script that starts all the SSH docker containers defined in
# `contrib/docker/ssh_server`. It's mainly to simplify running our SSH integration tests
# that disguise as unit tests.

#  - It first builds the images from the Dockerfile.* files in that directory if they are
#    either not present locally or outdated.
#  - Then it starts the command given by the user as command line arguments to the script.
#    This command is started with environment variables that advertise the docker containers.
#  - Finally - after the command finished - it shuts down the containers.
#
# It supports both docker and podman as container runtimes. podman is preferred if available.
#
# Example call: ./contrib/scripts/run_ssh_containers.sh ./build/bin/OrbitSshQtTests

set -euo pipefail

readonly REPO_ROOT=$(cd -- "$( dirname -- "${BASH_SOURCE[0]}" )/../../" &> /dev/null && pwd)
readonly IMAGE_DIR="${REPO_ROOT}/contrib/docker/ssh_server"

CONTAINERS=()
for dockerfile in $IMAGE_DIR/Dockerfile.*; do
  CONTAINERS+=(${dockerfile##*Dockerfile.})
done
readonly CONTAINERS

function container_runtime() {
  if which podman >/dev/null 2>&1; then
    podman $*
    return
  fi

  if which docker >/dev/null 2>&1; then
    docker $*
    return
  fi

  echo "No container runtime was found (tried podman and docker). Aborting!" >&2
  exit -1
}

function is_git_repo() {
  git -C "${REPO_ROOT}" status >/dev/null 2>&1
}

function is_clean_container_dir() {
  git -C "${REPO_ROOT}" diff --quiet HEAD -- "${IMAGE_DIR}"
}

function get_container_tag() {
  # If this is not a Git repo, we will always use the latest tag
  if ! is_git_repo; then
    echo "latest"
    return 0
  fi

  # If there are local changes to the docker image, we will also use the latest tag
  if ! is_clean_container_dir; then
    echo "latest"
    return 0
  fi

  # If there were no local changes to the Dockerfile and/or to other container input files
  # we can use the hash of the latest commit that made changes to these files as a cache
  # index.
  git log -1 --pretty=format:%h -- "${IMAGE_DIR}"
}

function does_image_needs_to_be_built() {
  local image="$1"
  local tag="$2"

  # If we are on the `latest` tag, there won't be any caching. We need to build the image.
  if [[ ${tag} == "latest" ]]; then
    return 0
  fi

  # No building needed, if we already have the image
  if container_runtime image inspect "${image}:${tag}" >/dev/null 2>&1; then
    return 1
  fi

  # Image not found. Building needed.
  return 0
}

function cleanup() {
  for variant in ${CONTAINERS[@]}; do
    container="ssh_server_${variant}"
    echo -n "Trying to stop container ${container}... "
    container_runtime stop ${container} >/dev/null || true
    echo "done."
  done
}

# Since we don't track which files in $IMAGE_DIR affect which container we will
# have the same image tag for all containers, so the tag can be defined as a constant here.
readonly TAG="$(get_container_tag)"

for variant in ${CONTAINERS[@]}; do
  image="ssh_server_${variant}"
  if does_image_needs_to_be_built ${image} ${TAG}; then
    echo "Building image for container ${image}:"
    container_runtime image build "${IMAGE_DIR}" -t ${image}:${TAG} -f "${IMAGE_DIR}/Dockerfile.${variant}"
    echo -e "-----\n"
  fi
done

trap "cleanup" EXIT

for variant in ${CONTAINERS[@]}; do
  image="ssh_server_${variant}"
  echo -n "Starting container ${image}: "
  container_runtime run -d --rm -P --name ${image} ${image}:${TAG}
  export ORBIT_TESTING_SSH_SERVER_${variant^^}_ADDRESS="127.0.0.1:$(container_runtime port ${image} | grep 0.0.0.0 | cut -f2 -d:)"
done

# Run whatever command the user provided
$*
