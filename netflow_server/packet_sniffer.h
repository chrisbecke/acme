#pragma once

namespace acme {

  class PacketSniffer
  {
  public:

    template<typename lambda_t>
    static int MonitorInterface(const char* interfaceName, lambda_t callback)
    {
      // Create a normal BSD raw socket
      auto sniffer = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
      if (sniffer == -1)
      {
        int err = errno; //WSAGetLastError();
        if (err == EACCES)
          printf("elevation failed?");
        else
          printf("Failed to create raw socket with error %d", err);

        return err;
      }
      assert(sniffer != -1);

#ifdef _WIN32
      sockaddr_in if0 = { 0 };
      if0.sin_family = AF_INET;
      if0.sin_addr.s_addr = inet_addr(interfaceName);

      auto r = bind(sniffer, (sockaddr*)&if0, sizeof(if0));
//      checksocketresult(r, "Error %d trying to bind to address");

      DWORD rcvall_flag = 1;
      DWORD bytesReturned = 0;
      r = WSAIoctl(sniffer, SIO_RCVALL, &rcvall_flag, sizeof(rcvall_flag), NULL, 0, &bytesReturned, NULL, NULL);
      if (r != 0)
      {
        SocketErrorCode err = LastSocketError();

        printf("Failed to make raw socket promicuous with error %d", err);

        return err;
      }
#endif
      char Buffer[65536];

      while (1)
      {
        // receive a snooped packet
        sockaddr from;
        socklen_t fromlen = sizeof(from);
        auto pktlen = recvfrom(sniffer, Buffer, sizeof(Buffer), 0, &from, &fromlen);
        internet_header* iphdr = (internet_header*)Buffer;

        callback(*iphdr, pktlen);
      }

      // unreachable

//      closesocket(sniffer);
      close(sniffer);
    }

  };


}
