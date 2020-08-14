#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Fail on any error.
DIR="/mnt/github/orbitprofiler"
SCRIPT="${DIR}/kokoro/checks/clang_format/check.sh"

if [ "$0" == "$SCRIPT" ]; then
  # We are inside the docker container

  echo -e "\n\nThe following files don't match our code-formatting standard:"
  echo -e "> This list includes all files in the repo that need formatting,"
  echo -e "> but changes outside of the scope of your PR won't affect the outcome"
  echo -e "> of this presubmit check.\n"
  while read line; do
    clang-format --output-replacements-xml $line --style=file| grep '<replacement ' > /dev/null
    if [ $? -eq 0 ]; then
      echo $line
    fi
  done <<< $(find "$DIR" -name '*.cpp' -o -name '*.h' \
        | grep -v "third_party/" \
        | grep -v "/build" )
  echo -e "--\n"

  cd "$DIR"
  REFERENCE="origin/master"
  MERGE_BASE="$(git merge-base $REFERENCE HEAD)" # Merge base is the commit on master this PR was branched from.
  FORMATTING_DIFF="$(git diff -U0 --no-color --relative $MERGE_BASE | clang-format-diff-9 --style=file -p1)"

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

else
  gcloud auth configure-docker --quiet
  docker run --rm --network host -v ${KOKORO_ARTIFACTS_DIR}:/mnt gcr.io/orbitprofiler/clang_format:latest $SCRIPT
fi

