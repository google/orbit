#!/bin/bash

cd "$( dirname "${BASH_SOURCE[0]}" )"

for i in llvm-common *; do
  (cd $i && conan export . orbitdeps/stable)
done
