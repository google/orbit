#!/bin/bash
find . \( -name '*.cpp' -o -name '*.h' \) ! -path './external/*' ! -path './build/*' ! -path './cmake-*' ! -name 'resource.h' | xargs clang-format -i
