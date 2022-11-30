#!/bin/bash
#
# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Fail on any error.
set -euo pipefail
shopt -s extglob

echo -e "\n\nThe following files don't match our code-formatting standard:"
echo -e "> This list includes all files in the repo that need formatting,"
echo -e "> but changes outside of the scope of your PR won't affect the outcome"
echo -e "> of this presubmit check.\n"
while read line; do
  if clang-format-14 --output-replacements-xml $line | grep '<replacement ' > /dev/null; then
    echo $line
  fi
done <<< $(find $GITHUB_WORKSPACE -name '*.cpp' -o -name '*.h' \
      | grep -v -P "third_party/(?!libunwindstack)" `# Remove all from third_party except libunwindstack`)
echo -e "--\n"

cd $GITHUB_WORKSPACE
# Use origin/main as reference branch, if not specified by Github
REFERENCE="origin/${GITHUB_BASE_REF:-main}"
MERGE_BASE="$(git merge-base $REFERENCE HEAD)" # Merge base is the commit on main this PR was branched from.
FORMATTING_DIFF="$(git diff -U0 --no-color --relative --diff-filter=r $MERGE_BASE -- 'third_party/libunwindstack/**' 'src/**' | clang-format-diff-14 -p1)"

git diff -U0 --no-color --relative --diff-filter=r $MERGE_BASE -- 'third_party/libunwindstack/**' 'src/**' | clang-format-diff-14 -p1 -i
git diff --minimal -U0 --no-color>> "${GITHUB_WORKSPACE}/clang_format.diff"

if [ -n "$FORMATTING_DIFF" ]; then
  echo "clang-format determined the following necessary changes to your PR:"
  echo "$FORMATTING_DIFF"
  echo -e "\n\nHere is the list of files that need formatting:"
  echo "$(echo "$FORMATTING_DIFF" | egrep '^\+\+\+' | cut -d' ' -f2 | cut -f1 )"
  echo -e "--\n\nNote: We recommend you to use git clang-format to format your changes!"
  exit 1
else
  echo "All your changes fulfill our code formatting requirements!"
  exit 0
fi