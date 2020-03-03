#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd )"




for profile in $@; do
  $DIR/check_outdated.sh $profile 2>/dev/null | while read package; do
    echo "Uploading $package..."
    conan upload -r bintray -c $package
  done
done

