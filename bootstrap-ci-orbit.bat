@echo off

SET REPO_ROOT=%~dp0

conan config install %REPO_ROOT%\contrib\conan\config
if %errorlevel% neq 0 exit /b %errorlevel%

call build.bat msvc2019_release_x64
