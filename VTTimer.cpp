#if defined(_MSC_VER) // Microsoft compiler
    #include "VTTimerWin.cpp"
#elif defined(__GNUC__) // GNU compiler
    #include "VTTimerLin.cpp"
#else
#error define your copiler
#endif