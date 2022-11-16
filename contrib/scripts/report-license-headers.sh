#!/bin/bash
#
# Copyright (c) 2022 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Fail on any error.
set -euo pipefail

function format_annotations() {
  local files_missing_license_headers=$1

  echo "{"
  echo "\"name\":\"missing-license-headers\","
  echo "\"head_sha\":$(echo -n "${COMMIT_SHA}" | jq -Rs .),"
  echo "\"status\":\"completed\","
  echo "\"conclusion\":\"failure\","
  echo "\"output\":{"
  echo "\"title\":\"Missing license headers report\","
  echo "\"summary\":\"You are missing the license headers in the following files.\","
  echo "\"annotations\":["
  FIRST_LINE=1
  while read -r line; do
    if [[ -z $FIRST_LINE ]]; then
      echo ","
    fi
    FIRST_LINE=""
    echo "{\"path\":$(echo -n "${line}" | jq -Rs .),"
    echo "\"start_line\":1,"
    echo "\"end_line\":1,"
    echo "\"start_column\":1,"
    echo "\"end_column\":1,"
    echo "\"message\":\"The source file must start with a license header.\","
    echo "\"annotation_level\":\"failure\","
    echo "\"title\":\"Missing license header\""
    echo "}"
  done <$files_missing_license_headers
  echo -n "]}}"
}

if [[ ! -s "${GITHUB_WORKSPACE}/missing_license_headers.txt" ]] ; then
  exit 0
fi

format_annotations "${GITHUB_WORKSPACE}/missing_license_headers.txt" | curl \
  -X POST \
  -H "Accept: application/vnd.github+json" \
  -H "Authorization: Bearer ${GITHUB_TOKEN}" \
  "https://api.github.com/repos/${GITHUB_REPOSITORY}/check-runs" \
  -d @-