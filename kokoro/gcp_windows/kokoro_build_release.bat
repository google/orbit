SET PACKAGE_DIR=%KOKORO_ARTIFACTS_DIR%\github\orbitprofiler\build_msvc2019_release_x64\package

call %KOKORO_ARTIFACTS_DIR%\github\orbitprofiler\kokoro\gcp_windows\kokoro_build.bat

cd %PACKAGE_DIR%
ren bin Orbit
zip -r Orbit.zip Orbit
