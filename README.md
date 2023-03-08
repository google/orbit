# ORBIT
![builds and tests](https://github.com/google/orbit/actions/workflows/build-and-test.yml/badge.svg?branch=main)
![Debian package](https://github.com/google/orbit/actions/workflows/release.yml/badge.svg?branch=main)
[![OpenSSF Scorecard](https://api.securityscorecards.dev/projects/github.com/google/orbit/badge)](https://api.securityscorecards.dev/projects/github.com/google/orbit)

<img alt="ORBIT Logo" src="contrib/logos/orbit_logo_simple.png" align="right" width="520" >

Orbit, the **O**pen **R**untime **B**inary **I**nstrumentation **T**ool is a
standalone **native** application profiler for Windows and Linux. It supports
native applications written in languages such as C, C++, Rust, or Go. Its main
purpose is to help developers identify the performance bottlenecks of a complex
application. Orbit can be also used to visualize the execution flow of such
applications.

The key differentiator with many existing tools is that no alteration to the
target process is necessary. Orbit does not require you to change a single line
of code. It doesn't require you to recompile or even relaunch the application
you want to profile. Everything is done seamlessly, right when you need it. It
requires zero integration time and zero iteration time.

Orbit combines sampling and dynamic instrumentation to optimize the profiling
workflow. Sampling can quickly identify interesting functions to instrument.
Dynamic instrumentation results in exact function entry and exit information
which is presented in the form of per-thread hierarchical call graphs.
Manual instrumentation markers can be added to the source code and further
allows for value-tracking. Scheduling events are also shown to visualize when a
thread was running and on what core. Furthermore, Orbit visualizes thread
dependencies, showing which thread got blocked or unblocked by which other
thread. For AMD GPUs, the submission, scheduling and hardware execution timings
of a job is visualized. Additional GPU data, such as Vulkan debug markers can be
retrieved using Orbit's Vulkan layer. Memory consumption and page-fault
information is visualized as well.

An introduction to Orbit's key features can be found in the following YouTube
video:
[![Orbit Presentation][orbit_youtube_presentation]](https://www.youtube.com/watch?v=8V-EPBPGZPs)

## Features

- Dynamic Instrumentation (no code change required)
- Callstack Sampling
- Wine/Proton Mixed-Callstack Profiling
- Thread Scheduling and Dependency Tracing
- Memory Tracing
- GPU Driver Tracepoints (AMD only)
- Vulkan Debug Label and Command Buffer Tracing (AMD only)
- Manual Instrumentation
- Source Code and Disassembly View
- Remote Profiling
- Debug Symbol Parsing (ELF, DWARF, PE and PDB)
- Full Serialization of Captured Data

### Note

Orbit's focus has shifted to the Linux version. Windows local profiling is
currently only supported partially and major features, such as dynamic
instrumentation, are not yet implemented. It is possible however to profile
Linux executables from a Windows UI instance. For Windows local profiling,
you can still use the released
[binaries](https://github.com/google/orbit/releases), but please note that
they are deprecated and mostly undocumented.

## Build

Please have a look at the first three sections of our
[development documentation](DEVELOPMENT.md). It describes how to build Orbit and
which compilers, platforms, and tools are supported and needed.

## Workflow

> **Note** An extensive documentation of the usage of Orbit can be found in our
> [usage documentation](documentation/DOCUMENTATION.md).

The following describes the basic workflow of Orbit:
1. Select a process in the list of currently running processes in the connection
   setup dialog, and click **Start Session**.
2. The list of loaded modules will appear at the top of the **Symbols** tab.
3. Orbit tries to automatically retrieve debug information of the modules.
   See [here](documentation/DOCUMENTATION.md#load-symbols) on how to load
   symbols for modules Orbit failed to load. For successfully loaded module
   symbols, the **Functions** tab will get populated.
4. Select functions you wish to dynamically instrument in the **Functions** tab
   by <kbd>Right-Click</kbd> and choosing **Hook**.
5. Start profiling by pressing <kbd>F5</kbd>. To stop profiling, press
   <kbd>F5</kbd> again. You can either zoom time using <kbd>W</kbd> and
   <kbd>S</kbd> or <kbd>Ctrl</kbd> + the scroll wheel. You can also
   <kbd>Ctrl</kbd>+<kbd>Right-Click</kbd> and drag to zoom to a specific time
   range. To scale the UI, press <kbd>Ctrl</kbd> + <kbd>+</kbd>/<kbd>-</kbd>.
   Press <kbd>SPACE</kbd> to see the last 2 seconds of capture.
6. You can select sections of the per-thread sampling event track to get a
   sampling report of your selection.

## Presets

Once you have loaded the debug information for your modules and have chosen
functions of interest to dynamically instrument, you can save your profiling
preset so that you won't have to do this manually again. To save a preset, go to
**File** > **Save Preset**

### Feedback

Questions and comments are more than welcome: please open an
[issue](https://github.com/google/orbit/issues/new).

## About

Orbit was created by [Pierric Gimmig](https://www.linkedin.com/in/pgimmig/), but
is now developed and maintained by a team of engineers at Google. The current
maintainers are:
* [@akopich](https://github.com/akopich)
* [@antonrohr](https://github.com/antonrohr)
* [@beckerhe](https://github.com/beckerhe)
* [@danielfenner](https://github.com/danielfenner)
* [@dimitry-](https://github.com/dimitry-)
* [@dpallotti](https://github.com/dpallotti)
* [@florian-kuebler](https://github.com/florian-kuebler)
* [@karupayun](https://github.com/karupayun)
* [@pierricgimmig](https://github.com/pierricgimmig)
* [@reichlfl](https://github.com/reichlfl)
* [@ronaldfw](https://github.com/ronaldfw)
* [@vickyliu-go4it](https://github.com/vickyliu-go4it)
* [@vwbaker](https://github.com/vwbaker)

## License

[License (BSD 2-clause)](./LICENSE)

<a href="http://opensource.org/licenses/BSD-2-Clause" target="_blank">
<img style="float: right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png" alt="OSI Approved License">
</a>

```text
Copyright (c) 2020 The Orbit Authors. All rights reserved.

BSD 2-Clause License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```


[orbit_youtube_presentation]: contrib/logos/orbit_presentation_youtube.png
