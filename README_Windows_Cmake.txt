
1. Install Qt (5.12.1 works).  Make sure you select the appropriate MSVC versions to install.
2. Add C:\Qt\5.12.1\msvc2017_64\bin\ to your PATH.  ("set PATH=%PATH%;C:\Qt\5.12.1\msvc2017_64\bin\")
2.1 Make sure you have the DIA SDK installed, (install Visual Studio with Windows 10 SDK component)
3. cd to root of orbitprofiler directory
4. mkdir build && cd build
5. cmake .. -A x64 (make sure to specify -A x64, otherwise it will try to build 32 bit...)
6. Open orbitProfiler.sln using a Visual Studio *started with administrator privileges*
7. Apply the temporary fixes:
- breakpad
Add orbitprofiler\external\vcpkg\buildtrees\breakpad\src\9e12edba6d-12269dd01c\src\processor\linked_ptr.h to orbitprofiler\external\vcpkg\installed\x64-windows\include\google_breakpad\processor
In orbitprofiler\external\vcpkg\installed\x64-windows\include\google_breakpad\processor\code_modules.h
	Replace 
	#include "processor/linked_ptr.h" 
	by 
	#include "google_breakpad/processor/linked_ptr.h"

8. Build solution
9. Manually copy these dlls to the .exe dir:
orbitprofiler\external\vcpkg\installed\x64-windows\debug\bin\libcurl-d.dll
orbitprofiler\external\vcpkg\installed\x64-windows\debug\bin\zlibd1.dll
orbitprofiler\build\OrbitDll\Release\OrbitDll.dll (rename to Orbit64.dll)
C:\Qt\5.12.1\msvc2017_64\bin\Qt5Core.dll
C:\Qt\5.12.1\msvc2017_64\bin\Qt5Gui.dll
C:\Qt\5.12.1\msvc2017_64\bin\Qt5Widgets.dll
NOTE: These steps are very temporary, the goal is to be able to build Orbit in a single step.