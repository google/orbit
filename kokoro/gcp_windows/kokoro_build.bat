:: Code under repo is checked out to %KOKORO_ARTIFACTS_DIR%\github.
:: The final directory name in this path is determined by the scm name specified
:: in the job configuration
SET REPO_ROOT=%~dp0

conan config install %REPO_ROOT%\contrib\conan\config
if %errorlevel% neq 0 exit /b %errorlevel%

cd %KOKORO_ARTIFACTS_DIR%\github\orbitprofiler
call build.bat msvc2019_release_x64
IF ERRORLEVEL 1 exit %ERRORLEVEL%

call conan package -bf %REPO_ROOT%\build_msvc2019_release_x64 %REPO_ROOT%
