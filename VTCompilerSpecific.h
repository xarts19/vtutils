#pragma once

#include <stddef.h>

// Detect compiler
#if defined(_MSC_VER)
#define COMPILER_MSVC
#elif defined(__GNUC__)
#define COMPILER_GCC
#else
#error "Unknown or unsupported compiler"
#endif

//
// MSVC++
//
#if defined(COMPILER_MSVC)

// TLS
#define THREAD_LOCAL __declspec(thread)

// Calling conventions
#define conv_cdecl    __cdecl
#define conv_stdcall  __stdcall
#define conv_thiscall __thiscall
#define conv_fastcall __fastcall

#define dll_export __declspec(dllexport)
#define dll_import __declspec(dllimport)

// x64 detection
#ifdef _M_AMD64
#define USE64
#else
#define USE32
#endif

//
// GCC
//
#elif defined(COMPILER_GCC)

#define COMPILER_GCC

// TLS
#define THREAD_LOCAL __thread

// Calling conventions
#define conv_cdecl
#define conv_stdcall
#define conv_thiscall
#define conv_fastcall

#define dll_export
#define dll_import

// x64 detection
#ifdef __LP64__
#define USE64
#else
#define USE32
#endif

#endif
