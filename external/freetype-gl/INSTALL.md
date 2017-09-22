# Install

## Ubuntu

The procedure is detailed in the [.travis.yml](.travis.yml).

If you want to reuse distribution packages as much as possible,
you need at the very least to:

-   compile and install AntTweakBar from source: <http://anttweakbar.sourceforge.net/doc/>

    There is no Ubuntu package for it, and that project is marked as unmaintained.

-   Ubuntu 15.10 and earlier required extra fixes to GLFW because of packaging and upstream bugs.

    Those were not present in 16.10 anymore, where you can just use:

        sudo apt-get install libglfw3-dev

    The fixes were:

    -   compile and install GLFW from source because of a Debian packaging bug with CMake:
        <https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=812853>

    -   apply the fix at
        <https://github.com/rougier/freetype-gl/blob/a4cfb9abac19a0ab62b625a9b6f856e032fe3732/.travis.yml#L23>
        to the installed GLFW files

Then:

    mkdir build
    cd build
    cmake ..
    make

You can then run some demos under:

    cd demos


## Windows MSYS2 MINGW64 with gcc 64-bit toolchain

<http://msys2.github.io/>

Be sure to check the MSYS2 wiki for install instructions / general information about the different shells etc. <https://sourceforge.net/p/msys2/wiki>
We set up only a 64-bit toolchain for the sake of brevity.

### Install required packages

Open up the MSYS2 shell e.g. with msys2.exe

```
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-make
pacman -S mingw-w64-x86_64-glew
pacman -S mingw-w64-x86_64-glfw
pacman -S mingw-w64-x86_64-fontconfig
pacman -S mingw-w64-x86_64-freetype
pacman -S mingw-w64-x86_64-harfbuzz
pacman -S mingw-w64-x86_64-pkg-config
pacman -S mingw-w64-x86_64-doxygen
```

### Generate Makefile

Open the MinGW64 shell e.g. via mingw64.exe
We need to explicitly tell CMake to generate MinGW Makefiles and enable harfbuzz examples.

```
mkdir build
cd build
cmake -G "MinGW Makefiles"  -Dfreetype-gl_BUILD_HARFBUZZ=ON ..
```

**Note**: Harfbuzz examples only work with symbolic links enabled. See <https://github.com/git-for-windows/git/wiki/Symbolic-links>

### Build demos

```
cmake --build .
```

### Run the demos

Go to the `demo/` folder to try some demos.
The harfbuzz examples are located in the `harfbuzz/` folder.

To run the `atb-agg` demo you need to copy the file `AntTweakBar64.dll` into the `demo/` folder.

### Troubleshooting
**Note**: If you have the installer ending in 20160921.exe then you have to manually create /mingw32 and /mingw64 directories in the msys2 installation directory.
This should be fixed with the next version of the installer.

`mkdir -p /mingw{32,64}`

Make sure to add your bin folder e.g. `C:\msys64\mingw64\bin`  to your PATH if you want to run the demos outside of the MINGW64 shell.

If you get an error when you start your application from the Windows Explorer like "The procedure entry point inflateReset2 could not be located in the dynamic link library zlib1.dll":
This is likely a PATH related problem. In this case some other zlib1.dll existent in one of the PATH folders was shadowing the needed zlib1.dll one of the mingw64/bin folder.
The solution is to change the order of the PATH entries so that the mingw64 folder comes first.

## macOS with homebrew

Install [homebrew](http://brew.sh/).

Then:

    brew install AntTweakBar ImageMagick cmake doxygen glfw3
    cmake .
    make

You can then run some demos under:

    cd demos
