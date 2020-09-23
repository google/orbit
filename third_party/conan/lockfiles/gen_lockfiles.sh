#!/bin/bash

REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd )"

if [[ $(uname -s) != Linux ]]; then
  echo "Generating the lockfiles on Windows is unfortunately broken"
  echo "at the moment due to a bug in the Qt package. Please use Linux"
  echo "for now"
  exit 1
fi

conan lock create ${REPO_ROOT}/conanfile.py \
  -o system_qt=False -o system_mesa=False  \
  -s os=Linux \
  --user=orbitdeps --channel=stable \
  --lockfile-out=${REPO_ROOT}/third_party/conan/lockfiles/base.lock \
  --base
