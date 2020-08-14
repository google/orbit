@echo off
set script=%0
call C:\cygwin64\bin\bash.exe -o igncr %script:.bat=.sh% %*
