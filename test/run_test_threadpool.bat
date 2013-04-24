call build_test_threadpool.bat

cd obj

VTThreadPoolTest.exe
if ERRORLEVEL 1 exit

cd ..
