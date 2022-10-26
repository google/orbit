#!/bin/bash
#
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# --- generate_coverage_report.sh ---
#
# This script generates a coverage report for the Orbit repository. It uses 
# llvm-cov and requires a build with build arguments -fprofile-instr-generate="%m.profraw"
# and -fcoverage-mapping. 
#
# usage: generate_coverage_report.sh SOURCE_DIR BUILD_DIR OUTPUT_DIR

# Settings:
# grep inverted matching list for directories in BUILD_DIR/src that are not used
BUILD_SRC_DIR_FILTER="ClientProtos\|CodeViewerDemo\|GrpcProtos"
# grep inverted matching list for files in BUILD_DIR/bin/ that are not used
BIN_FILE_FILTER="\.debug\|crashpad_handler"
# grep inverted matching list for source files that are not used
SOURCE_FILE_FILTER="Test.cpp\|Tests.cpp\|Fuzzer.cpp"
# ------------------------------------

# fail on any error
set -e

# Generate the css class string that determines the color of table cells
# parameter 1: percent; float, between 0 and 100
function get_css_class_string {
  local percent=$1
  local result="column-entry-red"
  if (( $(echo "$percent > 80" | bc) )); then
    result="column-entry-yellow"
  fi
  if (( $(echo "$percent >= 100" | bc) )); then
    result="column-entry-green"
  fi
  echo "$result"
}

# Generate a td html tag that contains the percent covered and is color coded
# parameter 1: count; int, total amount
# parameter 2: covered; int, amount covered (covered <= count)
function generate_html_td {
  local count=$1
  local covered=$2
  local percent=$(jq -n "$covered / $count * 100")
  local percent_rounded=$(printf "%0.2f" $percent)
  local css_class=$(get_css_class_string $percent_rounded)
  local percent_formatted=$(printf "%6s" "$percent_rounded")
  echo "<td class='$css_class'><pre> $percent_formatted% ($covered/$count)</pre></td>"
}

if [ "$#" -ne 3 ]; then
  echo "error: wrong number of arguments."
  echo "usage: generate_coverage_report.sh SOURCE_DIR BUILD_DIR OUTPUT_DIR"
  exit 1
fi

SOURCE_DIR=$(realpath $1)
if [ ! -d $SOURCE_DIR ]; then
  echo "error: directory $1 does not exist."
  exit 1
fi

BUILD_DIR=$(realpath $2)
if [ ! -d $BUILD_DIR ]; then
  echo "error: directory $2 does not exist."
  exit 1
fi

OUTPUT_DIR=$(realpath $3)
if [ ! -d $OUTPUT_DIR ]; then
  echo "error: directory $3 does not exist."
  exit 1
fi

BINS=$(find $BUILD_DIR/bin/ -maxdepth 1 -type f | grep -v "$BIN_FILE_FILTER")
FORMATTED_BINARY_FILES=$(echo $BINS | sed "s/ / -object=/g")
SUM_FUNCTIONS_COUNT=0
SUM_FUNCTIONS_COVERED=0
SUM_LINES_COUNT=0
SUM_LINES_COVERED=0
SUM_REGIONS_COUNT=0
SUM_REGIONS_COVERED=0

for COMPONENT_DIR in $(ls -d $BUILD_DIR/src/*/ | grep -v "$BUILD_SRC_DIR_FILTER")
do
  COMPONENT=$(basename $COMPONENT_DIR)
  if ls $COMPONENT_DIR/*.profraw 1> /dev/null 2>&1; then
    echo "$COMPONENT: preprocessing"
  else 
    echo "$COMPONENT: skipping (No Unit Tests)"
    HTML_TABLE+="<tr class='light-row'><td><pre>$COMPONENT</pre></td><td colspan='3'><pre>No Unit Tests</pre></td></tr>"
    continue
  fi

  llvm-profdata-11 merge -sparse \
    -o $BUILD_DIR/$COMPONENT.profdata \
    $COMPONENT_DIR/*.profraw 

  SOURCE_FILES=$(find $SOURCE_DIR/$COMPONENT -regex ".*\.\(h\|cpp\)" | grep -v "$SOURCE_FILE_FILTER")

  echo "$COMPONENT: generating html report"
  llvm-cov-11 show -format=html \
    -show-line-counts-or-regions -show-instantiations=false \
    -output-dir=$OUTPUT_DIR/coverage_$COMPONENT \
    -instr-profile=$BUILD_DIR/$COMPONENT.profdata \
    $FORMATTED_BINARY_FILES \
    $SOURCE_FILES

  echo "$COMPONENT: generating json summary"
  JSON=$(llvm-cov-11 export -summary-only \
    -instr-profile=$BUILD_DIR/$COMPONENT.profdata \
    $FORMATTED_BINARY_FILES $SOURCE_FILES 2>/dev/null \
    | jq -c '.data[0].totals')

  FUNCTIONS_COUNT=$(jq -c '.functions.count' <<< $JSON)
  ((SUM_FUNCTIONS_COUNT+=FUNCTIONS_COUNT))
  FUNCTIONS_COVERED=$(jq -c '.functions.covered' <<< $JSON)
  ((SUM_FUNCTIONS_COVERED+=FUNCTIONS_COVERED))
  FUNCTIONS_TD=$(generate_html_td $FUNCTIONS_COUNT $FUNCTIONS_COVERED)

  LINES_COUNT=$(jq -c '.lines.count' <<< $JSON)
  ((SUM_LINES_COUNT+=LINES_COUNT))
  LINES_COVERED=$(jq -c '.lines.covered' <<< $JSON)
  ((SUM_LINES_COVERED+=LINES_COVERED))
  LINES_TD=$(generate_html_td $LINES_COUNT $LINES_COVERED)

  REGIONS_COUNT=$(jq -c '.regions.count' <<< $JSON)
  ((SUM_REGIONS_COUNT+=REGIONS_COUNT))
  REGIONS_COVERED=$(jq -c '.regions.covered' <<< $JSON)
  ((SUM_REGIONS_COVERED+=REGIONS_COVERED))
  REGIONS_TD=$(generate_html_td $REGIONS_COUNT $REGIONS_COVERED)

  HTML_TABLE+="<tr class='light-row'><td><pre><a href='coverage_$COMPONENT/index.html'>$COMPONENT</a></pre></td>$FUNCTIONS_TD$LINES_TD$REGIONS_TD</tr>"
done

echo "Generating html summary"
FORMATTED_DATE="$(date +'%Y-%m-%d %H:%M')"
INDEX_HTML="<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'><meta charset='UTF-8'><link rel='stylesheet' type='text/css' href='coverage_OrbitQt/style.css'></head><body><h2>Coverage Report</h2><h4>Created: $FORMATTED_DATE</h4><p>Click <a href='http://clang.llvm.org/docs/SourceBasedCodeCoverage.html#interpreting-reports'>here</a> for information about interpreting this report.</p><div class='centered'><table><tr><td class='column-entry-bold'>Component</td><td class='column-entry-bold'>Function Coverage</td><td class='column-entry-bold'>Line Coverage</td><td class='column-entry-bold'>Region Coverage</td></tr>"
INDEX_HTML+=$HTML_TABLE
INDEX_HTML+="<tr class='light-row-bold'><td><pre>Totals</pre></td>$(generate_html_td $SUM_FUNCTIONS_COUNT $SUM_FUNCTIONS_COVERED)$(generate_html_td $SUM_LINES_COUNT $SUM_LINES_COVERED)$(generate_html_td $SUM_REGIONS_COUNT $SUM_REGIONS_COVERED)</tr></table></body></html>"

echo "$INDEX_HTML" > $OUTPUT_DIR/index.html

echo "Success! Coverage report generated at $OUTPUT_DIR/index.html"

exit 0
