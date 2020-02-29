# Getting started with development

## Platforms

Windows 10 and Linux are supported.

## Compilers

To build Orbit you need a compiler capable of C++17. The following ones should be fine.
You should prefer clang over GCC, since most of the developers build with clang by default.

* GCC 8 and above on Linux
* Clang 7 and above on Linux
* MSVC 2017, 2019 and above on Windows

## Building Orbit

Orbit relies on `conan` as its package manager.  Conan is written in Python3,
so make sure you have either Conan installed or at least have Python3 installed.

The `bootstrap-orbit.{sh,bat}` will try to install `conan` via `pip3` if not
installed and reachable via `PATH`. Afterwards it calls `build.{sh,bat}` which
will compile Orbit for you.

On Linux, `python3` should be preinstalled anyway, but you might need to install
pip (package name: `python3-pip`).

On Windows, one option to install Python is via the Visual Studio Installer.
Alternatively you can download prebuilts from [python.org](https://www.python.org/)
(In both cases verify that `pip3.exe` is in the path, otherwise the bootstrap
script will not be able to install conan for you.)

## Dependencies

All our third party library and dependencies are either included via
git submodules or they are managed by conan.

There are some exceptions. On Linux, we rely by default on the distribution's Qt5
and Mesa installation. This can be changed by modifying the conan package options
`system_qt` and `system_mesa`.

The simplest way to do that is to change the default values of these two options.
Check out `conanfile.py`. There is a python dictionary called `default_options`
defined in the python class `OrbitConan`.

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

_Note:_ Since the GGP SDK is not publicly available, this only works inside
of Google, at least for now.

Call the script `bootstrap-orbit-ggp.sh` which creates a package out of the GGP
SDK (you do not need to have the SDK installed), and compiles Orbit against
the toolchain from the GGP SDK package.

Finally, `build_ggp_release/package/bin/OrbitService` can be copied over
to the instance:
```bash
ggp ssh put build_ggp_release/package/bin/OrbitService
```

Some libraries still need to be installed on the instance:
```bash
sudo apt install libglu1-mesa libxi6 libxmu6
```

before the service can be started with:
```bash
ggp ssh shell
> sudo /mnt/developer/OrbitService
```
