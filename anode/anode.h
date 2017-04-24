#pragma once
// This header file must be included first by the anode application
// It sets up the anode execution environment.
#include "core/string.hpp"
#include "core/console.hpp"

// Pull in the platform SDK files
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <Windows.h>
#endif

// If Visual C++, then patch up the entry point to be "main()"
#ifdef _MSC_VER
#ifndef _WINDLL
#if _UNICODE
#pragma comment(linker,"/ENTRY:wmainCRTStartup")
#define main wmain
#else
#pragma comment(linker,"/ENTRY:mainCRTStartup")
#endif
#endif
#endif


