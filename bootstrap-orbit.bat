@echo off

SET REPO_ROOT=%~dp0

where /q conan
if ERRORLEVEL 1 (
    echo "Conan not found. Trying to install it via python-pip..."

    where /q pip3
    if ERRORLEVEL 1 (
        echo "It seems you don't have Python3 installed (pip3.exe not found)."
        echo "Please install Python or make it available in the path."
        echo "Alternatively you could also just install conan manually and make"
        echo "it available in the path."
	GOTO :EOF
    )

    pip3 install conan
    if ERRORLEVEL 1 GOTO :EOF

    where /q conan
    if ERRORLEVEL 1 (
        echo "It seems we installed conan sucessfully, but it is not available"
        echo "in the path. Please ensure that your Python user-executable folder is"
        echo "in the path and call this script again."
        echo "You can call 'pip3 show -f conan' to figure out where conan.exe was placed."
        GOTO :EOF
    )
) else (
    echo "Conan found. Installation skipped."
)

conan config install %REPO_ROOT%\contrib\conan\configs\windows
if ERRORLEVEL 1 GOTO :EOF

call build.bat %*
