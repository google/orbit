#!/bin/bash

set -e

REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd )"

export CONAN_USER_HOME=""
trap "{ if [ -d "$CONAN_USER_HOME" ]; then rm -rf $CONAN_USER_HOME; fi; }" EXIT

let total_packages_missing=0 || true

for server in internal public; do
  for os in windows linux; do
    export CONAN_USER_HOME="$(mktemp -d)"

    if [ "$os" == "linux" ]; then
      build_profile="clang7_relwithdebinfo"
    else
      build_profile="msvc2017_relwithdebinfo"
    fi

    install_options="--assume-$os"
    build_profile_path="$REPO_ROOT/third_party/conan/configs/$os/profiles/$build_profile"

    if [ "$server" == "public" ]; then
      install_options="$install_options --force-public-remotes"
    fi

    set -e
    $REPO_ROOT/third_party/conan/configs/install.sh $install_options >$CONAN_USER_HOME/stdout 2>$CONAN_USER_HOME/stderr
    RETURN_CODE=$?

    if [ $RETURN_CODE -ne 0 ]; then
      echo "Error installing conan configuration. Exit code: $RETURN_CODE STDOUT:" >&2
      cat <$CONAN_USER_HOME/stdout >&2
      rm -f $CONAN_USER_HOME/stdout
 
      echo -e "\n STDERR:"
      cat <$CONAN_USER_HOME/stderr >&2
      rm -f $CONAN_USER_HOME/stderr
      exit $RETURN_CODE
    fi
  
    set +e

    for lockfile_path in $REPO_ROOT/third_party/conan/lockfiles/$os/*/conan.lock; do
      profile="$(basename $(dirname $lockfile_path))" 
      profile_path="$REPO_ROOT/third_party/conan/configs/$os/profiles/$profile"
  
      echo -n "Checking $os/$profile with $server remote(s)...  " >&2
      json_raw="$(conan info -u -pr:b "$build_profile_path" -pr:h "$profile_path" -l $lockfile_path $REPO_ROOT -j 2>$CONAN_USER_HOME/stderr)"
      RETURN_CODE=$?

      if [ $RETURN_CODE -ne 0 ]; then 
        echo -e "Failed.\nconan info failed. Exit code: $RETURN_CODE\n  Stderr:" >&2
        cat <$CONAN_USER_HOME/stderr >&2
        echo -e "  Stdout:\n$json_raw" >&2
        exit $RETURN_CODE
      fi

      rm -f $CONAN_USER_HOME/stderr
  
      missing_packages="$(echo "$json_raw" | grep reference | jq '.[] | select(.is_ref) | select(.binary != "Cache" and .binary != "Download" and .binary != "Skip") | .reference + "#" + .revision' | tr -d '"')" 
  
      lines=$(echo -n "$missing_packages" | wc -l)
  
      if [ $lines -ne 0 ]; then
        echo "$lines package(s) is/are missing:" >&2
        echo -e "$missing_packages\n\n" | sed -e 's/^/\t/'
        let total_packages_missing+=$lines
      else
        echo "No packages are missing!" >&2
      fi
  
    done

    if [ -d "$CONAN_USER_HOME" ]; then
      rm -rf "$CONAN_USER_HOME"
      unset CONAN_USER_HOME
    fi
  done
done

if [ $total_packages_missing -ne 0 ]; then
  exit 1
fi
