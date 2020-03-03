#!/bin/bash

REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd )"

for profile in $@; do
  echo "Checking profile $profile..." >&2
  PACKAGES=$(conan info -pr $profile $REPO_ROOT -j | grep build_id | jq '.[] | select(.is_ref) | .reference + ":" + .id ' | grep -v 'ggp_sdk/' | grep -v 'llvm/' | grep -v 'cmake/' | tr -d '"')

  for package in $PACKAGES; do
    reference=$(echo $package | awk -F ':' '{ print $1 }')
    echo $reference | grep "@" > /dev/null
    if [ $? -ne 0 ]; then
      reference="$reference@"
    fi

    package_id=$(echo $package | awk -F ':' '{ print $2 }')
    # echo "Checking if package $package is outdated..." >&2
    conan search -r bintray --raw --outdated $reference | grep $package_id > /dev/null
    if [ $? -eq 0 ]; then
      echo $package
    fi
  done
done

