call build_tests.bat

cd obj

VTCPPLoggerTest.exe
if ERRORLEVEL 1 exit
REM VTPrettyPrintTest.exe
REM if ERRORLEVEL 1 exit
VTThreadPoolTest.exe
if ERRORLEVEL 1 exit

cd ..
