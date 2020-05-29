#!/bin/bash
find . \( -name '*.cpp' -o -name '*.h' \) ! -path './third_party/*' ! -path './build/*' ! -path './cmake-*' ! -name 'resource.h' | xargs clang-format -i
