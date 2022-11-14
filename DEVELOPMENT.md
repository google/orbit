# Getting started with development

Orbit consists of two parts - the frontend and the collector, also called the service.
The collector is responsible for instrumenting the target process and recording
profiling events which are then streamed to the frontend, also called the UI.

The communication between frontend and collector is handled by a [gRPC](https://grpc.io/)
connection. gRPC uses HTTP 2.0 as its base communication layer. When talking to a Stadia
instance we wrap that once more into an SSH tunnel.

## Platforms

The frontend is supported on Windows 10 and Linux. The collector currently
only works on Linux.

Previous versions of Orbit supported profiling on Windows, but due to the
priority shift towards Stadia this support is currently in a non-working state.
There are plans to bring it back, but at this point we can't commit to any time
schedule.

If you want to try profiling on Windows, we recommend you to download the older 1.0.2
release from GitHub's [releases page](https://github.com/google/orbit/releases).

## Compilers

To build Orbit you need a compiler capable of C++17. The following ones should be fine.

- GCC 9 and above on Linux
- Clang 7 and above on Linux
- MSVC 2019 and above on Windows (MSVC 2017 is no longer supported by some dependencies)

## Dependencies

All our third-party libraries and dependencies are managed by conan.

### Qt on Linux

There are some exceptions. On Linux, we rely by default on the distribution's Qt5
and Mesa installation. This can be changed by modifying the conan package options
`system_qt` and `system_mesa`, but we recommend to go with the distribution provided
Qt package. You will need at least version 5.12.4 of Qt. The point release is important
because it resolves a [known issue](https://bugreports.qt.io/browse/QTBUG-69683). Note,
that Ubuntu 18.04 LTS comes with Qt 5.9 which is NOT sufficient. Ubuntu 20.04 LTS is
fine though.

In case you still want to have Qt provided by conan, the simplest way to do that will be
to change the default values of these two options.
Check out `conanfile.py`. There is a python dictionary called `default_options`
defined in the python class `OrbitConan`.

### Qt on Windows

On Windows you have the choice to either let Conan compile Qt from source or use
one of the prebuilt distributions from [The Qt Company](https://qt.io/). (Note that
as of writing this, you need to register to download the distribution packages.)

We recommend to use a prebuilt distribution since compiling from source can take
several hours.

If you decide to compile from source, you don't have to prepare anything.
You can skip over the next paragraph and go to "Building Orbit".

If you decide to use a prebuilt Qt distribution, please download and install it
yourself. Keep in mind the prebuilt has to match your Visual Studio version and
architecture. You also have to install the QtWebEngine component which is usually
not selected by default in the installer.

As of writing this the minimum supported Qt version is 5.12.4 but this might change.
We recommend the version which we also compile from source. It can be found by checking
`conanfile.py`. Search for `self.requires("qt/`. The version can be found after that string.

As a next step you have to tell the Orbit build system where Qt is installed by
setting an environment variable `Qt5_DIR`. It has to point to the directory
containing the CMake config file `Qt5Config.cmake`. For Qt 5.15.0 installed to
the default location the path is `C:\Qt\5.15.0\msvc2019_64\lib\cmake\Qt5`;

You don't have to set that variable globally. It's fine to set it in a local
PowerShell when starting the bootstrapping script:

```powershell
$Env:Qt5_DIR="C:\Qt\5.15.0\msvc2019_64\lib\cmake\Qt5"
.\bootstrap-orbit.ps1
```

The value of `Qt5_DIR` is persisted in the default conan profiles. You can
call `conan profile show default_relwithdebinfo` (after running bootstrap)
to see the value of `Qt5_DIR`.

If you have pre-existing `default_*`-profiles and want to switch to prebuilt
Qt distributions you have to either delete these profiles - the build script
will regenerate them - or you can manually edit them.

A default profile with Qt compiled from source is pretty much empty:

```
# default_relwithdebinfo

include(msvc2019_relwithdebinfo)

[settings]
[options]
[build_requires]
[env]

```

A default profile prepared for a prebuilt Qt package has two extra lines:

```
# default_relwithdebinfo

include(msvc2019_relwithdebinfo)

[settings]
[options]
OrbitProfiler:system_qt=True
[build_requires]
[env]
OrbitProfiler:Qt5_DIR="C:\Qt\5.15.0\msvc2019_64\lib\cmake\Qt5"

```

You can find all the conan profiles in `%USERPROFILE%\.conan\profiles`.

## Building Orbit

Orbit relies on `conan` as its package manager. Conan is written in Python3,
so make sure you have either Conan installed or at least have Python3 installed.

The `bootstrap-orbit.{sh,ps1}` will try to install `conan` via `pip3` if not
installed and reachable via `PATH`. Afterwards it calls `build.{sh,ps1}` which
will compile Orbit for you.

On Linux, `python3` should be preinstalled anyway, but you might need to install
pip (package name: `python3-pip`).

On Windows, one option to install Python is via the Visual Studio Installer.
Alternatively you can download prebuilts from [python.org](https://www.python.org/)
(In both cases verify that `pip3.exe` is in the path, otherwise the bootstrap
script will not be able to install conan for you.)

## Running Orbit

Like mentioned before, the collector currently only works for Linux. So the following
only applies there:

1. Start Orbit via 
```bash
./build_default_relwithdebinfo/bin/Orbit
```
2. Start OrbitService by clicking the button `Start OrbitService`. To obtain scheduling
   information, the collector needs to run as root, hence this will prompt you for a
	 password (via [pkexec](https://linux.die.net/man/1/pkexec)). Alternatively, you can 
	 start OrbitService yourself:

```bash
sudo ./build_default_relwithdebinfo/bin/OrbitService # Start the collector
```

The frontend currently has no graphical user interface to connect to a generic
remote instance. Only Stadia is supported as a special case. 

If you needed remote profiling support you could tunnel the mentioned TCP port through
a SSH connection to an arbitrary Linux server. There are plans on adding generic
SSH tunneling support but we can't promise any timeframe for that.

## Consistent code styling

We use `clang-format` to achieve a consistent code styling across
the whole code base. You need at least version 7.0.0 of `clang-format`.

Please ensure that you applied `clang-format` to all your
files in your pull request. Otherwise a presubmit check will fail
and unfortunately only Googlers have access to the detailed log.

On Windows, we recommend getting `clang-format` directly from the
LLVM.org website. They offer binary packages of `clang`, where
`clang-format` is part of.

Visual Studio 2017+ ships `clang-format` as part of the IDE though.
(https://devblogs.microsoft.com/cppblog/clangformat-support-in-visual-studio-2017-15-7-preview-1/)

On most Linux distributions, there is a dedicated package called `clang-format`.

Most modern IDEs provide `clang-format` integration via either an extension
or directly.

A `.clang-format` file which defines our specific code style lives in the
top level directory of the repository. The style is identical to the Google
style. 

## Code Style

As mentioned above we use `clang-format` to enforce certain aspects of code
style. The Google C++ style guide we are following in that can be found
[here](https://google.github.io/styleguide/cppguide.html). It includes brief
discussions or rationales for all the style decisions.

Beyond what it is in the style guide we agreed to a few more additional rules
specific to the Orbit project:

### [[nodiscard]]
We use `[[nodiscard]]` for (almost) all new class methods and free functions
that return a value. If you encouter a use case where it makes no sense or
hurts readability feel free to skip it though.

We do not touch existing code merely to add `[[nodiscard]]` though.

### Error handling
For error handling we use `ErrorMessageOr<T>` from 
[Result.h](https://github.com/google/orbit/blob/main/src/OrbitBase/include/OrbitBase/Result.h).
This class serves the same purpose as `absl::StatusOr<T>`. We thought about
switching to absl but currently the advantage does not seem large enough to
warrant the effort. So for now we stick with `ErrorMessageOr<T>`. 

In cases where no error message needs to be returned it is perfectly fine to
use `std::optional`.

### Exceptions
Currently our code is compiled with exceptions but we strive towards a world
with no exceptions. Particularly we don't use methods from std that throw
exceptions but prefer the variants returning error codes (e.g. in 
`std::filesystem`).

### Namespaces
We place all code belonging to a module named `ModuleName` or `OrbitModuleName`
inside the top-level namespace `orbit_module_name`. All namespaces start with
the `orbit_` prefix. We do not use nested namespaces.

Exclusively for code that needs to be in a `.h` file public to a module (i.e.,
in the `include` directory) even if that code shouldn't be used by other
modules, we use the top-level namespace `orbit_module_name_internal` instead.
This is *not* nested inside `orbit_module_name`.

### File system structure
Each module gets its own subdirectory under `src/`. The subdirectory's name
should be spelled in camel case, i.e. `src/ModuleName`. All public header
files go into a separate include subdirectory `include/ModuleName`, i.e.
the full path looks like `src/ModuleName/include/ModuleName/PublicHeader.h`.

Note that the module's name appears twice in this path.
They will be included relative to the `include/` subdirectory, i.e.
`#include "ModuleName/PublicHeader.h"`.

CPP files and private header files go directly into the module's subdirectory,
i.e. `src/ModuleName/MyClass.cpp`.

### Tests
Unit test files go into the root directory of the module. They should be named after
the component they are testing - followed by the suffix `Test`,
i.e. `src/ModuleName/MyClassTest.cpp`.

### Fuzzers
Similar to unit tests, fuzzers are also named after the component they are fuzzing
with the suffix `Fuzzer`, i.e. `src/ModuleName/MyClassFuzzer.cpp`.

### Platform-specific code
We try to keep platform-specific code out of header files and maintain a
platform-agnostic header for inclusion. The platform-specific implementations
go into separate files, suffixed by the platform name, i.e. `MyClassWindows.cpp`
or `MyClassLinux.cpp`.

It's not always possible to keep the header entirely platform-agnostic. If some
distinctions need to be made, you can use preprocessor macros, e.g.:

```
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif
```

## FAQ

### What's the difference between `bootstrap-orbit.{sh,ps1}` and `build.{sh,ps1}`?

`bootstrap-orbit.{sh,ps1}` performs all the tasks which have to be done once per developer machine.
This includes:

- Installing system dependencies
- Installing the correct version of conan if necessary.
- Installing the conan configuration (which changes rarely).

Afterwards `bootstrap-orbit.{sh,ps1}` calls `build.{sh,ps1}`.

`build.{sh,ps1}` on the other hand performs all the tasks which have to be done
once per build configuration. It creates a build directory, named after the
given conan profile, installs the conan-managed dependencies into this build folder,
calls `cmake` for build configuration and starts the build.

Whenever the dependencies change you have to call `build.{sh,ps1}` again.
A dependency change might be introduced by a pull from upstream or by a switch
to a different branch.

It might occur to you that even though you called `build.{sh,ps1}` your build still
fails with a weird error message. Most of the time this is due to outdated but cached
information managed by CMake. The simplest way to resolve that problem is to make
a clean build by deleting the build directory and calling `build.{sh,ps1}`.

`build.{sh,ps1}` can initialize as many build configurations as you like from the
same invocation. Just pass conan profile names as command line arguments. Example for Linux:

```bash
./build.sh clang7_debug gcc9_release clang9_relwithdebinfo ggp_release
# is equivalent to
./build.sh clang7_debug
./build.sh gcc9_release
./build.sh clang9_relwithdebinfo
./build.sh ggp_release
```

### Calling `build.{sh,ps1}` after every one-line-change takes forever! What should I do?

`build.{sh,ps1}` is not meant for incremental builds. It should be called only once to initialize
a build directory. (Check out the previous section for more information on what `build.{sh,ps1}`
does.)

For incremental builds, switch to the build directory and ask cmake to run the build:

```bash
cd <build_folder>/
cmake --build . # On Linux
cmake --build . --config {Release,RelWithDebInfo,Debug} # On Windows
```

Alternatively, you can also just call `make` on Linux. Check out the next section on how to
enable building with `ninja`.

### How do I enable `ninja` for my build?

> **Note:** Linux only for now! On Windows you have to use MSBuild.

If you want to use `ninja`, you cannot rely on the `build.sh` script, which automatically
initializes your build with `make` and that cannot be changed easily later on.
So create a build directory from scratch, install the conan-managed dependencies
and invoke `cmake` manually. Here is how it works:

Let's assume you want to build Orbit in debug mode with `clang-9`:

```bash
mkdir build_clang9_debug # Create a build directory; should not exist before.
cd build_clang9_debug/
conan install -pr clang9_debug ../ # Install conan-managed dependencies
cp ../third_party/toolchains/toolchain-linux-clang9-debug.cmake toolchain.cmake # Copy the cmake toolchain file, which matches the conan profile.
cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake -G Ninja ../
ninja
```

> ### Note:
>
> Please be aware that it is your responsibility to ensure that the conan profile is compatible
> with the toolchain parameters cmake uses. In this example clang9 in debug mode is used in both cases.

#### Another example without toolchain files:

You can also manually pass the toolchain options to cmake via the command line:

```bash
mkdir build_clang9_debug # Create a build directory; should not exist before.
cd build_clang9_debug/
conan install -pr clang9_debug ../ # Install conan-managed dependencies
cmake -DCMAKE_CXX_COMPILER=clang++-9 -DCMAKE_C_COMPILER=clang-9 -DCMAKE_BUILD_TYPE=Debug -G Ninja ../
ninja
```

### How do I integrate with CLion?

In CLion, the IDE itself manages your build configurations, which is incompatible with our `build.{sh,ps1}`
script. That means, you have to manually install conan dependencies into CLion's build directories
and you have to manually verify that the directory's build configuration matches the used conan profile
in terms of compiler, standard library, build type, target platform, etc.

```bash
cd build_directory_created_by_clion/
conan install -pr matching_conan_profile ..
```

After that, you can trigger a rerun of `cmake` from CLion and it will now be able to pick up all the missing
dependencies.

This process can be automated by the [CLion Conan Extension](https://plugins.jetbrains.com/plugin/11956-conan).

After installing the extension, it will take care of installing conan dependencies into your
CLion build directories, whenever necessary. The only task left to you is to create a mapping
between CLion build configurations and conan profiles. Check out
[this blog post](https://blog.jetbrains.com/clion/2019/05/getting-started-with-the-conan-clion-plugin)
on how to do it.

Add `-DCMAKE_CXX_FLAGS=-fsized-deallocation` to Settings -> Build, Execution, Deployment -> CMake -> CMake options.
This flag is needed because Orbit's codebase makes use of C++14's [sized-deallocation feature](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3536.html)
which is not enabled by default on the clang compiler.

### How do I integrate conan with Visual Studio?

Visual Studio has this concept of having multiple build configurations in the same build directory.
This concept is not very wide-spread on buildsystems coming from the Unix world. Both CMake and
Conan have support for it, but some of our dependencies currently do not support it.

That means, **currently you can't have debug and release builds in the same build folder!**
Please ensure that Visual Studio is set to the build configuration which matches your build-
folder's conan profile. Non-matching build configurations will result in a lot of linker errors.

There is a [Conan Extension for Visual Studio](https://marketplace.visualstudio.com/items?itemName=conan-io.conan-vs-extension),
which is currently under development, but should be able to help you, when you develop on
Visual Studio.

### How do I integrate with Visual Studio Code?

Visual Studio Code uses configuration files to specify tasks that can be executed. These files are provided in the `contrib/.vscode` folder. To enable building from Visual Studio Code, simply copy the whole folder into the root directory with the name `.vscode`:

```bash
cp -r contrib/vscode .vscode
```

### The build worked fine, but when I try to call cmake manually I get `cmake not found!`

Conan installs cmake as a build dependency automatically, but won't make it available in the PATH.

If you want to use conan's cmake installation, you can use the `virtualenv` generator to create
a virtual environment which has `cmake` in its PATH:

```bash
cd my_build_folder/
conan install -pr my_conan_profile -g virtualenv ../
source ./activate.sh # On Linux (bash) or on Windows in git-bash
.\activate.bat # On Windows in cmd
.\activate.ps1 # On Windows in powershell
cmake ... # CMake from conan is now available
```

There is also a `deactivate.{sh,bat,ps1}` which make your shell leave the virtual environment.

### `ERROR: .../orbitprofiler/conanfile.py: 'options.ggp' doesn't exist` ?!?

This message or a similar one indicates that your build profiles are
outdated and need to be updated. You can either just call the bootstrap
script again or you can manually update your conan config:

```bash
conan config install third_party/conan/configs/[windows,linux]
```

### How can I use separate debugging symbols for Linux binaries?

Orbit supports loading symbols from your workstation. Simply add directories that contain debugging symbols to the `SymbolPaths.txt` file. This file can be found at

- Windows: `C:\Users\<user>\AppData\Roaming\OrbitProfiler\config\SymbolPaths.txt`
- Linux: `~/orbitprofiler/config/SymbolPaths.txt`

The symbols file must named in one of three ways. The same fname as the binary (`game.elf`), the same name plus the `.debug` extension (`game.elf.debug`) or the same name but the `.debug` extension instead of the original one (`game.debug`). To make sure the binary and symbols file have been produced in the same build, Orbit checks that they have a matching build id.

## Cross-Compiling for GGP

Cross compilation is supported on Windows and Linux host systems.

_Note:_ Cross compiling the UI is not supported.

_Note:_ Since the GGP SDK is not publicly available, this only works inside
of Google, at least for now.

Call the script `bootstrap-orbit-ggp.{sh,ps1}` which creates a package out of the GGP
SDK (you do not need to have the SDK installed for this to work, but you will need it
for deployment), and compiles Orbit against the toolchain from the GGP SDK package.

Finally, `build_ggp_release/package/bin/OrbitService` can be copied over
to the instance:

```bash
ggp ssh put build_ggp_release/package/bin/OrbitService /mnt/developer/
```

before the service can be started with:

```bash
ggp ssh shell
> sudo /mnt/developer/OrbitService
```
