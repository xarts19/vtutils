set -e

mkdir obj
cd obj

g++ -Wall -Wextra -std=c++0x ../VTCPPLoggerTest.cpp ../../VTThreadWin.cpp ../../VTCPPLogger.cpp ../../VTLockWin.cpp ../../VTUtilWin.cpp -o VTCPPLoggerTest

cd ..
