set ORBIT_VERSION=1.0.2
set TARGET_NAME=ORBIT_PROFILER_%ORBIT_VERSION%
set TARGET_PATH=builds\%TARGET_NAME%\
set PLATFO_PATH=%TARGET_PATH%platforms\
set SHADER_PATH=%TARGET_PATH%text\
set LICENS_PATH=%TARGET_PATH%licenses\
set SYMSTORE="C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\symstore.exe"
set PDB_PATH=OrbitPdb\%ORBIT_VERSION%\
set ZIP_PATH=OrbitWeb\App_Data\download\
set ORBIT_PATH_64=build\x64\OrbitQt\
set ORBIT_PATH_32=build\x86\OrbitDll\

@RD /S /Q %TARGET_PATH%

mkdir %TARGET_PATH%
mkdir %PLATFO_PATH%
mkdir %SHADER_PATH%
mkdir %LICENS_PATH%

::Orbit
xcopy %ORBIT_PATH_64%Release\Orbit.exe                %TARGET_PATH%
xcopy %ORBIT_PATH_64%Release\Orbit64.dll              %TARGET_PATH%
xcopy %ORBIT_PATH_32%Release\Orbit32.dll              %TARGET_PATH%
xcopy Orbit.h                                         %TARGET_PATH%

::Curl
xcopy bin\x64\Release\libcurl.dll                     %TARGET_PATH%

::DIA
xcopy bin\x64\Release\msdia140.dll                    %TARGET_PATH%

::Licenses
xcopy licenses\*.*                                    %LICENS_PATH%

::ReleaseNotes
xcopy release_notes.txt                               %TARGET_PATH%
                                                      
::Qt                                                  
xcopy bin\x64\Release\Qt5Core.dll                     %TARGET_PATH%
xcopy bin\x64\Release\Qt5Widgets.dll                  %TARGET_PATH%
xcopy bin\x64\Release\Qt5Gui.dll                      %TARGET_PATH%
xcopy bin\x64\Release\platforms\qwindows.dll          %PLATFO_PATH%

::Icon
xcopy OrbitQt\Orbit.ico                               %TARGET_PATH%
                                                      
::Glew                                                
xcopy bin\x64\Release\glew32.dll                      %TARGET_PATH%

::Text
xcopy "external\freetype-gl\fonts\Vera.ttf"           %SHADER_PATH%
xcopy "external\freetype-gl\shaders\v3f-t2f-c4f.vert" %SHADER_PATH%
xcopy "external\freetype-gl\shaders\v3f-t2f-c4f.frag" %SHADER_PATH%

::License
xcopy license.txt                                     %TARGET_PATH%

::Docs
xcopy README.txt                                      %TARGET_PATH%

::Zip
cd builds
..\external\7-Zip\7z a ..\%ZIP_PATH%%TARGET_NAME%.zip %TARGET_NAME%\
cd ..

::SymStore
%SYMSTORE% add /3 /f "bin\x64\Release\Orbit.exe" /s OrbitSymbols /t Orbit
%SYMSTORE% add /3 /f "bin\x64\Release\Orbit.pdb" /s OrbitSymbols /t Orbit

::Explorer
start %ZIP_PATH%
