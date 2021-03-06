#pragma once

// This makes the MSVC toolchain expect the correct ("main") entrypoint
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

// System header files for socket using console apps on windows
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <Mstcpip.h>
#include <Ws2tcpip.h> // getaddrinfo etc
#endif

// system header files for socket using console apps on *nix
#ifndef _MSC_VER
#include <unistd.h> // close
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <errno.h> // errno, EWHATEVER
#include <arpa/inet.h> // inet_addr


//#ifdef _POSIX_VERSION
//#endif

#endif
