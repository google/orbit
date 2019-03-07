Requirements:
- Make sure you have the DIA SDK installed, (install Visual Studio with Windows 10 SDK component)
-- ex: C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\DIA SDK

1. Install Qt (5.12.1 works).  Make sure you select the appropriate MSVC versions to install.
2. Create a QTDIR environment variable and set it to your qt installation directory (ex: C:\Qt\5.12.1\msvc2017_64\bin\).
3. cd to root of orbitprofiler directory
4. bootstrap-orbit.sh
6. Open build/orbitProfiler.sln using a Visual Studio *started with administrator privileges*
7. Build solution