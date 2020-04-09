#!/bin/bash

# Fail on any error.
set -ex

DIR="/mnt/github/orbitprofiler"
SCRIPT="${DIR}/kokoro/gcp_ubuntu/kokoro_build.sh"

if [ "$0" == "$SCRIPT" ]; then
  # We are inside the docker container

  echo "Installing conan configuration (profiles, settings, etc.)..."
  conan config install "${DIR}/contrib/conan/configs/linux"
  cp -v "${DIR}/kokoro/conan/config/remotes.json" ~/.conan/remotes.json

  export CONAN_REVISIONS_ENABLED=1
  CONAN_PROFILE="clang7_release"

  # PACKAGES=$(conan info -pr $CONAN_PROFILE . -j 2>/dev/null | grep build_id | \
  #         jq '.[] | select(.is_ref) | .reference' | grep -v 'llvm/' | tr -d '"')

  conan install -u -pr ${CONAN_PROFILE} -if "${DIR}/build_${CONAN_PROFILE}/" \
          --build outdated "${DIR}"
  conan build -bf "${DIR}/build_${CONAN_PROFILE}/" "${DIR}"
  conan package -bf "${DIR}/build_${CONAN_PROFILE}/" "${DIR}"

  # echo -n "${PACKAGES}" | while read package; do
  #   conan upload -r artifactory --all --no-overwrite all --confirm $package
  # done

  # Uncomment the three lines below to print the external ip into the log and
  # keep the vm alive for two hours. This is useful to debug build failures that
  # can not be resolved by looking into sponge alone. Also comment out the
  # "set -e" at the top of this file (otherwise a failed build will exit this
  # script immediately).
  # external_ip=$(curl -s -H "Metadata-Flavor: Google" http://metadata/computeMetadata/v1/instance/network-interfaces/0/access-configs/0/external-ip)
  # echo "INSTANCE_EXTERNAL_IP=${external_ip}"
  # sleep 7200;

else
  gcloud auth configure-docker --quiet
  docker run --rm --network host -v ${KOKORO_ARTIFACTS_DIR}:/mnt gcr.io/orbitprofiler/clang7_opengl_qt:latest $SCRIPT
fi

