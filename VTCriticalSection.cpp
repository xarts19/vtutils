#if defined(_MSC_VER) // Microsoft compiler
    #include "VTCriticalSectionWin.cpp"
#elif defined(__GNUC__) // GNU compiler
    #include "VTCriticalSectionLin.cpp"
#else
#error define your copiler
#endif