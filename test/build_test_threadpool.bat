call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\bin\vcvars32.bat"

MD obj
cd obj

cl /EHsc /nologo /W4 /Zi ..\VTThreadPoolTest.cpp ..\..\VTThreadPool.cpp ..\..\VTThreadWin.cpp ..\..\VTEventWin.cpp ..\..\VTLockWin.cpp ..\..\VTUtilWin.cpp ..\..\VTTimerWin.cpp
if ERRORLEVEL 1 exit

cd ..
