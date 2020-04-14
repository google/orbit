SET PACKAGE_DIR=%KOKORO_ARTIFACTS_DIR%\github\orbitprofiler\build\package

call %KOKORO_ARTIFACTS_DIR%\github\orbitprofiler\kokoro\gcp_windows\kokoro_build.bat

cd %PACKAGE_DIR%
ren bin Orbit
zip -r Orbit.zip Orbit
