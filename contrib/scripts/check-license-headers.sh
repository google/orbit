#!/bin/bash
#
# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Fail on any error.
set -euo pipefail

cd $GITHUB_WORKSPACE
# Use origin/main as reference branch, if not specified by Github
REFERENCE="origin/${GITHUB_BASE_REF:-main}"
MERGE_BASE="$(git merge-base $REFERENCE HEAD)" # Merge base is the commit on main this PR was branched from.
LICENSE_HEADER_MISSED=""

echo -e "The following files are missing a license/copyright header:\n"
while read file; do
  if [ ! -f "$file" ]; then
    continue
  fi

  if ! grep -E 'Copyright \(c\) [0-9]{4} The Orbit Authors\. All rights reserved\.' "$file" > /dev/null; then
    LICENSE_HEADER_MISSED="yes"
    echo "$file"
  fi

done <<< $(git diff -U0 --no-color --relative --name-only --diff-filter=r $MERGE_BASE \
| grep -v third_party/ \
| grep -v /build \
| egrep '\.cpp$|\.h$|CMakeLists\.txt$|\.js$|\.proto$|\.py$')

if [ -n "$LICENSE_HEADER_MISSED" ]; then
  exit 1
fi