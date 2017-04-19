// netflow_server.cpp : Defines the entry point for the console application.
//

#include "netflow_server.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _MSC_VER
#ifdef _WIN32
#pragma comment (lib, "Ws2_32.lib")
#endif
#endif

#ifdef _WIN32

void WindowsInitSockets()
{
  WSADATA wsaData;

  auto err = WSAStartup(MAKEWORD(2, 2), &wsaData);
  assert(!err);
}

void WindowsShutdownSockets()
{
  WSACleanup();
}

void WindowsTestForRawSocketSupport()
{
  const int bufferSize = 0x4000;
  LPWSAPROTOCOL_INFO protocolInfo = (LPWSAPROTOCOL_INFO)malloc(bufferSize);

  DWORD bufferLen = bufferSize;
  auto protocolCount = WSAEnumProtocols(NULL, protocolInfo, &bufferLen);
  
  free(protocolInfo);
}


#endif


int main(int argc, char* argv[])
{
  WindowsInitSockets();
  WindowsTestForRawSocketSupport();

  // Create a normal BSD raw socket
  auto sniffer = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
  if (sniffer == INVALID_SOCKET)
  {
    int err = WSAGetLastError();
    if (err == WSAEACCES)
      printf("elevation failed?");
    else
      printf("Failed to create raw socket with error %d",err);

    return err;
  }
  assert(sniffer != INVALID_SOCKET);

  sockaddr_in if0 = { 0 };
//  memcpy(&dest.sin_addr.s_addr, local->h_addr_list[in],
    //sizeof(dest.sin_addr.s_addr));
  if0.sin_family = AF_INET;
  if0.sin_port = 0;
//  dest.sin_zero = 0;
  int r = bind(sniffer, (sockaddr*)&if0, sizeof(if0));
  assert(r == 0);

  DWORD rcvall_flag = 1;
  DWORD bytesReturned = 0;
  r = WSAIoctl(sniffer, SIO_RCVALL, &rcvall_flag, sizeof(rcvall_flag), NULL, 0, &bytesReturned, NULL, NULL);
  if (r != 0)
  {
    int err = WSAGetLastError();

    printf("Failed to make raw socket promicuous with error %d", err);

    return err;
  }

  char Buffer[65536];

  while (1)
  {
    sockaddr from;
    int fromlen = sizeof(from);
    int pktlen = recvfrom(sniffer, Buffer, sizeof(Buffer), 0, &from, &fromlen);
  }


  closesocket(sniffer);

  WindowsShutdownSockets();
	return 0;
}

