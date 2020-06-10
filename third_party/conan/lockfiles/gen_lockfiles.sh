#!/bin/bash

REPO_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../../../" >/dev/null 2>&1 && pwd )"

if [ "$(uname -s)" == "Linux" ]; then
  subdirectory="linux"
  profiles=({ggp,clang{7,8,9},gcc{8,9}}_{release,relwithdebinfo,debug})
else
  subdirectory="window"
  profiles=({ggp,msvc{2017,2019}}_{release,relwithdebinfo,debug})
fi

for profile in ${profiles[@]}; do
  tmpfile="$(mktemp -d)"
  conan graph lock "$REPO_ROOT" -pr $profile "--lockfile=$tmpfile" || exit $?
  jq --indent 1 'del(.graph_lock.nodes."0".path)' < "$tmpfile/conan.lock" \
    > "$REPO_ROOT/third_party/conan/lockfiles/$subdirectory/$profile/conan.lock" || exit $?
  rm -rf "$tmpfile"
done
