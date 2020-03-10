#!/bin/bash

# Fail on any error.
set -e

# Display commands being run.
# WARNING: please only enable 'set -x' if necessary for debugging, and be very
#  careful if you handle credentials (e.g. from Keystore) with 'set -x':
#  statements like "export VAR=$(cat /tmp/keystore/credentials)" will result in
#  the credentials being printed in build logs.
#  Additionally, recursive invocation with credentials as command-line
#  parameters, will print the full command, with credentials, in the build logs.
set -x


# Code under repo is checked out to ${KOKORO_ARTIFACTS_DIR}/github.
# The final directory name in this path is determined by the scm name specified
# in the job configuration.
cd ${KOKORO_ARTIFACTS_DIR}/github/orbitprofiler
./bootstrap-ci-orbit.sh

# Uncomment the three lines below to print the external ip into the log and
# keep the vm alive for two hours. This is useful to debug build failures that
# can not be resolved by looking into sponge alone. Also comment out the
# "set -e" at the top of this file (otherwise a failed build will exit this
# script immediately).
# external_ip=$(curl -s -H "Metadata-Flavor: Google" http://metadata/computeMetadata/v1/instance/network-interfaces/0/access-configs/0/external-ip)
# echo "INSTANCE_EXTERNAL_IP=${external_ip}"
# sleep 7200;
