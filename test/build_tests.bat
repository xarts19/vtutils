call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\bin\vcvars32.bat"

MD obj
cd obj

cl /EHsc /nologo /W4 /Zi ..\VTCPPLoggerTest.cpp ..\..\VTThreadWin.cpp ..\..\VTCPPLogger.cpp ..\..\VTLockWin.cpp ..\..\VTUtilWin.cpp
REM if ERRORLEVEL 1 exit

REM cl /EHsc /nologo /W4 ..\VTPrettyPrintTest.cpp
REM if ERRORLEVEL 1 exit

cl /EHsc /nologo /W4 /Zi ..\VTThreadPoolTest.cpp ..\..\VTThreadPool.cpp ..\..\VTThreadWin.cpp ..\..\VTEventWin.cpp ..\..\VTLockWin.cpp ..\..\VTUtilWin.cpp ..\..\VTTimerWin.cpp
if ERRORLEVEL 1 exit

cd ..
