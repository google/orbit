#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo "Installing conan configuration (profiles, settings, etc.)..."
conan config install $DIR/contrib/conan/config || exit $?

exec $DIR/build.sh default_release

