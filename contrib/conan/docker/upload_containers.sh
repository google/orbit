#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

for profile in {clang{7,8,9},gcc{8,9},ggp}_{release,relwithdebinfo,debug}; do
  docker push gcr.io/orbitprofiler/$profile:latest || exit $?
done
