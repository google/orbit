# Freetype GL - A C OpenGL Freetype engine

[![Build Status Travis](https://travis-ci.org/rougier/freetype-gl.png?branch=master)](https://travis-ci.org/rougier/freetype-gl)
[![Build Status Appveyor](https://ci.appveyor.com/api/projects/status/github/rougier/freetype-gl?branch=master)](https://ci.appveyor.com/project/rougier/freetype-gl)

A small library for displaying Unicode in OpenGL using a single texture and
a single vertex buffer.

![Screenshot](http://raw.github.com/rougier/freetype-gl/master/doc/images/markup.png)

[Installation instructions](INSTALL.md).

## Code organization

### Mandatory files

* **texture-font**:  The texture-font structure is in charge of creating bitmap
                     glyphs and to upload them to the texture atlas.

* **texture-atlas**: This structure is responsible for the packing of small
                     regions into a bigger texture. It is based on the skyline
                     bottom left algorithm which appear to be well suited for
                     storing glyphs. More information at:
                     http://clb.demon.fi/files/RectangleBinPack.pdf

* **vector**:        This structure loosely mimics the std::vector class from
                     c++. It is used by texture-atlas (for storing nodes),
                     texture-font (for storing glyphs) and font-manager (for
                     storing fonts). More information at:
                     http://www.cppreference.com/wiki/container/vector/start


### Optional files

* **markup**:        Simple structure that describes text properties (font
                     family, font size, colors, underline, etc.)

* **font-manager**:  Structure in charge of caching fonts.

* **vertex-buffer**: Generic vertex buffer structure inspired by pyglet
                     (python). (more information at http://www.pyglet.org)

* **edtaa3func**:    Distance field computation by Stefan Gustavson
                     (more information at http://contourtextures.wikidot.com/)

* **makefont**:      Allow to generate header file with font information
                     (texture + glyphs) such that it can be used without
                     freetype.


## Contributors

* Ryan.H.Kawicki (Initial CMake project)
* Julian Mayer (Several bugfixes and code for demo-opengl-4.cc)
* Sylvain Duclos (Android port)
* Wang Yongcong (Improvements on the windows build and code review)
* Jonas Wielicki (Bug report & fix on the CMakefile)
* whatmannerofburgeristhis (Bug report in makefont)
* Andrei Petrovici (Fine analysis of the whole code and report of potential problems)
* Cristi Caloghera (Report on bad vertex buffer usage)
* Andrei Petrovici (Code review)
* Kim Jacobsen (Bug report & fix)
* bsoddd (Bug report & fix)
* Greg Douglas (Bug report & fix)
* Jim Teeuwen (Bug report & fix)
* quarnster (Bug report & fix)
* Per Inge Mathisen (Bug report & fix)
* Wojciech Mamrak (Code review, bug report & fix)
* Wael Eloraiby (Put code to the C89 norm and fix CMakefile)
* Christian Forfang (Code review, fix & patch for 3.2 core profile)
* Lukas Murmann (Code review & fix for 3.2 core profile)
* Jérémie Roy (Code review, fix and new ideas)
* dsewtz (Bug report & fix)
* jcgamestoy (Bug report & fix)
* Behdad Esfahbod (Bug fix on harfbuzz demo)
* Marcel Metz (Bug report & fix, CMmake no demo option, makefont parameters)
* PJ O'Halloran (svn to git migration)
* William Light (Face creation from memory)
* Jan Niklas Hasse (Bug report & fix + README.md)
* Pierre-Emmanuel Lallemant (Bug report & fix + travis setup)
* Robert Conde (Bug report & fix)
* Mikołaj Siedlarek (Build system bug fix)
* Preet Desai (Bug report & fix)
* Andy Staton (CMake fix and added namespace safeguard (avoiding glm collisions))
* Daniel Burke (Removed GLEW dependency and fix problems with font licences)
* Bob Kocisko (Added horizontal text alignment and text bounds calculation)
* Ciro Santilli (Improve markdown documentation)
