@echo off

setlocal enabledelayedexpansion

SET REPO_ROOT=%~dp0

set argCount=0
for %%x in (%*) do (
   set /A argCount+=1
   set "argVec[!argCount!]=%%~x"
)

if %argCount% == 0 (
    set /a argCount=2
    set argVec[1]=default_release_x86
    set argVec[2]=default_release_x64
)


for /L %%i in (1,1,%argCount%) do (
    set profile=!argVec[%%i]!

    set defaultprofile=NO
    if "!profile!" == "default_release_x86" set defaultprofile=YES
    if "!profile!" == "default_release_x64" set defaultprofile=YES

    if "!defaultprofile!" == "YES" (
        call :conan_profile_exists !profile!
        if !errorlevel! neq 0 (
            echo "Creating conan profile !profile!"
            call :create_conan_profile !profile!
        )
    )

    echo Building Orbit in build_!profile!/ with conan profile !profile!

    call conan install -pr !profile! -o system_qt=False -if build_!profile!/ --build outdated !REPO_ROOT!
    if !errorlevel! neq 0 exit /b !errorlevel!

    call conan build -bf build_!profile!/ !REPO_ROOT!
    if !errorlevel! neq 0 exit /b !errorlevel!

    call conan package -bf build_!profile!/ !REPO_ROOT!
    if !errorlevel! neq 0 exit /b !errorlevel!
)

EXIT /B 0

:create_conan_profile
    conan profile new --detect %~1

    IF "%~1" == "default_release_x86" (
        conan profile update settings.arch=x86 %~1
    ) ELSE (
        conan profile update settings.arch=x86_64 %~1
    )

	powershell -Command "(gc !USERPROFILE!/.conan/profiles/%~1) -replace '\[build_requires\]', \"[build_requires]`r`ncmake/3.16.4@\" | Out-File -encoding ASCII !USERPROFILE!/.conan/profiles/%~1"

    EXIT /B 0
Rem End of create_conan_profile

:conan_profile_exists
    conan profile show %~1 >NUL 2>&1
    EXIT /B !errorlevel!
Rem End of conan_profile_exists