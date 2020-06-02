#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ "$(uname -s)" == "Linux" ]; then
  for profile in {clang{7,8,9},gcc{8,9},ggp}_{release,relwithdebinfo,debug}; do
    docker push gcr.io/orbitprofiler/$profile:latest || exit $?
  done
else
  for profile in msvc{2017,2019}_{release,relwithdebinfo,debug}; do
    docker push gcr.io/orbitprofiler/$profile:latest || exit $?
  done
fi
