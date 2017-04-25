// netflow_server.cpp : Defines the entry point for the console application.
//
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "netflow_server.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "tcpip.h"
#include "dgram.h"

#include "packet_sniffer.h"
#include "netflow _packet_builder.h"

using namespace acme;
using namespace acme::dgram;

#ifdef _MSC_VER
#ifdef _WIN32
#pragma comment (lib, "Ws2_32.lib")
#endif
#endif

#ifdef _WIN32

/*
enum SocketErrorCode {
  wsa_invalid = 10022
};
*/

inline SocketErrorCode LastSocketError()
{
  return  (SocketErrorCode)WSAGetLastError();
}


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

void checksocketresult(int r, const char* message)
{
  if (r == 0)
    return;

#ifdef _WIN32
  socketerror err = LastSocketError();
#else
  socketerror err = socketerror(errno);
#endif

  printf(message, err);

  assert(0);
}




void print_tcp(const internet_header& hdr, const tcp_header& tcp, int len)
{
  char src[20];
  char dst[20];
  strcpy(src, inet_ntoa(*((in_addr*)&hdr.source_address)));
  strcpy(dst, inet_ntoa(*((in_addr*)&hdr.destination_address)));

  printf("%15s %5d/tcp -> %15s %5d/tcp %d\r\n",
    src, htons(tcp.source_port), dst, htons(tcp.dest_port),len);
}

void print_udp(const internet_header& hdr, const udp_header& udp, int len)
{
  char src[20];
  char dst[20];
  strcpy(src, inet_ntoa(*((in_addr*)&hdr.source_address)));
  strcpy(dst, inet_ntoa(*((in_addr*)&hdr.destination_address)));

  printf("%15s %5d/udp -> %15s %5d/udp %d\r\n",
    src, htons(udp.source_port), dst, htons(udp.dest_port),len);
}

// unknown
void print_ip(const internet_header& hdr, int len)
{
  char src[20];
  char dst[20];
  strcpy(src, inet_ntoa(*((in_addr*)&hdr.source_address)));
  strcpy(dst, inet_ntoa(*((in_addr*)&hdr.destination_address)));

  char const* protocol;
  switch (hdr.protocol)
  {
  case 1: protocol = "ICMP"; break;
  case 2: protocol = "IGMP"; break;
  case 6: protocol = "TCP"; break;
  case 17: protocol = "UDP"; break;
  default:
    protocol = "XXXX";
  }

  printf("%d %d %02d %5d | %04x %05d | %3d %4s %04x | %15s | %15s | %d - %d\r\n",
    hdr.version, hdr.ihl, hdr.tos, ntohs(hdr.total_len),   // 1
    hdr.id, ntohs(hdr.frag_off),                           // 2
    hdr.ttl, protocol, hdr.checksum,            // 3
    src,                                            // 4
    dst,                                            // 5
    hdr.options,
    len);                                   // 6
}


void print(const internet_header& hdr, int len)
{
  const char* ip_payload = (const char*)((uint32_t*)&hdr + hdr.ihl);

  switch (hdr.protocol)
  {
  case 17: print_udp(hdr, *(udp_header*)ip_payload, len); break;
  case 6: print_tcp(hdr, *(tcp_header*)ip_payload, len); break;
  default:
  case 2:  print_ip(hdr, len); break;
  }
}


int main(int argc, char* argv[])
{
  printf("netflow_server starting...\r\n");
#ifdef _WIN32
  WindowsInitSockets();
  WindowsTestForRawSocketSupport();
#endif

  NetflowPacketBuilder packetBuilder;

  //auto test = dgram::createSocket(AF_INET);
  //auto test = dgram::Socket(AF_INET);
  //test.bind();
  //auto address = Address::resolve("netbox", "2055", AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  //test.SendTo("hello world", 100,address);

  auto udp = dgram::Socket(AF_INET);
  udp.bind();

  // The netflow collector is running here.
  Address addr = Address::resolve("localhost", "9996", AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  int result = PacketSniffer::MonitorInterface("192.168.1.107", [&](const internet_header& header,int len){
    print(header, len);

    packetBuilder.AddFlow(header, len, [&](void* data, int len){
      // packet builder calls this when it thinks the collected flows must be sent.
      udp.SendTo(data,len,addr);
    });

  });

#ifdef _WIN32
  WindowsShutdownSockets();
#endif
  return result;
}

