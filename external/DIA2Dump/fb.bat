@echo off
%comspec% /c ""C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64 8.1 && "fbuild.exe" %*"