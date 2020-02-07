:: Code under repo is checked out to %KOKORO_ARTIFACTS_DIR%\github.
:: The final directory name in this path is determined by the scm name specified
:: in the job configuration
cd %KOKORO_ARTIFACTS_DIR%\github\orbitprofiler

call bootstrap-orbit.bat
exit %ERRORLEVEL%
