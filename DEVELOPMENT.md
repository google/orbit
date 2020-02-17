# Getting started with development

## Building Orbit

Orbit relies on CMake as its meta build system in a fairly recent version
(3.15 at least). This won't be available on most stable Linux distributions.

One way to install a recent version of CMake is via Python's pip. This
script should install CMake on debian-based distributions. For other
flavors you might need to adjust the commands to your package manager:

```bash
# Put ~/.local/bin into your PATH with high precedence (high means BEFORE /usr/bin).
sudo apt install python3-pip
pip3 install --user --upgrade pip
which pip # Should point to ~/.local/bin/pip
pip install --user cmake
which cmake # Should point to ~/.local/bin/cmake
cmake --version # Should output 3.15.2 or higher
```

On Windows, this should work as well. But installing via the Qt Installer or
via the Windows Installer from cmake.org are also viable options.

Check out the `bootstrap-orbit.{sh, bat}` file for a general description on how
to install dependencies and on how to set up a `build/`-directory.

## Dependencies

All our 3rd party dependencies are either included via git submodules or they are
managed by vcpkg. But there are several dependencies vcpkg needs to build our
dependencies from source. This especially applies to Linux where it is expected
to have several libraries or tools to be provided by the system. Check out
`bootstrap-orbit.sh` for more information.

On Windows you need to provide a Visual Studio installation with the DIA SDK
installed.

Under Linux we use a custom triplet for vcpkg. This triplet takes care to build
some dependencies dynamically and some statically. (At the moment qt5 is built
dynamically while all the rest is built statically. Check
`contrib/vcpkg/triplets/x64-linux-mixed.cmake` to be sure if this information
is still up to date.) When calling `vcpkg` manually be sure to specify the
correct triplet (`--triplet x64-linux-mixed`) and you have to point `vcpkg` to
the triplet location directory
(`--overlay-triplets=$PROJECT_ROOT/contrib/vcpkg/triplets/`). You can check out
the `bootstrap-orbit.sh` file on how it is done.


## Consistent code styling

We use `clang-format` to achieve a consistent code styling across
the whole code base. You need at least version 7.0.0 of `clang-format`.

Please ensure that you applied `clang-format` to all your
files in your pull request.

On Windows, we recommend getting `clang-format` directly from the
LLVM.org website. They offer binary packages of `clang`, where
`clang-format` is part of.

Visual Studio 2017 ships `clang-format` as part of the IDE.
(https://devblogs.microsoft.com/cppblog/clangformat-support-in-visual-studio-2017-15-7-preview-1/)

On most Linux distributions, there is a dedicated package called `clang-format`.

Most modern IDEs provide `clang-format` integration via either an extension
or directly.

A `.clang-format` file which defines our specific code style lives in the
top level directory of the repository. The style is identical to the Google
style.

## Cross-Compiling for GGP

_Note:_ This was only tested on Linux. Cross compilation on Windows
is currently not supported. There are some issues with cross-compiling
openssl.

_Note:_ Cross compiling the UI is not supported.

First we need to cross compile all the dependencies:
```bash
cd external/vcpkg
./vcpkg --overlay-triplets=../../contrib/vcpkg/triplets/ \
  --triplet x64-linux-ggp install abseil freetype freetype-gl breakpad \
  capstone asio cereal imgui freeglut glew curl gtest
cd ../../
```

In a second step, we can compile Orbit itself:
```bash
mkdir build_ggp_release
cd build_ggp_release
cp ../contrib/toolchains/toolchain-linux-ggp-release.cmake toolchain.cmake
cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -G Ninja ..
ninja # or cmake --build .
```

Finally, `build_ggp_release/OrbitService/OrbitService` can be copied over
to the instance:
```bash
ggp ssh put OrbitService/OrbitService .
```

Some dependencies still need to be installed on the instance:
```bash
sudo apt install libglu1-mesa libxi6 libxmu6
```
