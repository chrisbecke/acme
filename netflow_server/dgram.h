#pragma once

namespace acme {

  enum class socketerror
  {
#ifdef _WIN32
    Invalid           =  WSAEINVAL,        // 10022
    AddrNotAvailable  =  WSAEADDRNOTAVAIL, // 10049
#elif defined POSIX
    Invalid           = 0,
    AddrNotAvailable  = EADDRNOTAVAIL
#endif
  };

  namespace dgram {

    enum class SocketType
    {
      udp4 = 2,   // AF_INET
      udp6 = 23   // AF_INET6
    };

    socketerror err()
    {
      return (socketerror)WSAGetLastError();
    }

    class Address
    {
      sockaddr* _addr;
    public:
      int len;
    public:
      Address(sockaddr* rhs, int len){
        _addr = (sockaddr*)malloc(len);
        this->len = len;
        memcpy(_addr, rhs, len);
      }
      Address(const Address& rhs){
        _addr = (sockaddr*)malloc(rhs.len);
        len = rhs.len;
        memcpy(_addr, rhs._addr,len);
      }
      Address(Address&& rhs){
        _addr = rhs._addr;
        rhs._addr = nullptr;
        len = rhs.len;
      }
      ~Address()
      {
        if (_addr)
          free(_addr);
      }
      Address& operator=(const Address& rhs){
        _addr = (sockaddr*)malloc(rhs.len);
        len = rhs.len;
        memcpy(_addr, rhs._addr,len);
        return *this;
      }
      Address& operator=(Address&& rhs){
        _addr = rhs._addr;
        rhs._addr = nullptr;
        len = rhs.len;
        return *this;
      }
      sockaddr* operator*(){
        return _addr;
      }
      const sockaddr* operator*() const{
        return _addr;
      }

      operator bool() const
      {
        return _addr != nullptr;
      }
      static Address resolve(const char* hostname, const char* service, int family=AF_UNSPEC, int socktype=0,int protocol=0)
      {
        struct addrinfo hints, *servinfo;
        int r;

        memset(&hints, 0, sizeof hints);

        hints.ai_family = family; // AF_UNSPEC; // AF_INET | AF_INET6
        hints.ai_socktype = socktype; // SOCK_DGRAM;
        hints.ai_protocol = protocol; // IPPROTO_UDP;

        r = getaddrinfo(hostname, service, &hints, &servinfo);
        assert(!r);

        Address address(servinfo->ai_addr, servinfo->ai_addrlen);

        freeaddrinfo(servinfo); // all done with this structure

        return address;

      }
    };

    class Socket
    {
      int _fd;

    public:
      Socket(int af):_fd(-1){
        _fd = socket(af, SOCK_DGRAM, IPPROTO_UDP);
      }
      ~Socket()
      {
        if (_fd >= 0)
          closesocket(_fd); 
      }

      void bind(uint16_t port=0)
      {
        sockaddr_in addr = { 0 };
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        int r = ::bind(_fd, (const sockaddr*)&addr, sizeof(addr));
        assert(!r);
      }

      void bind(const char* hostname, const char* service=nullptr)
      {
        struct addrinfo hints, *servinfo, *p;
        int r;

        memset(&hints, 0, sizeof hints);

        hints.ai_family = AF_UNSPEC; // AF_INET | AF_INET6
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;

        r = getaddrinfo(hostname, service, &hints, &servinfo);
        assert(!r);

        for (p = servinfo; p != NULL; p = p->ai_next)
        {
          p->ai_addr;
          r = ::bind(_fd, p->ai_addr, p->ai_addrlen);
          assert(!r);
          break;
        }

        freeaddrinfo(servinfo); // all done with this structure
      }

      int SendTo(const void* data, int len, const Address& to)
      {
        sockaddr_in* addr = (sockaddr_in*)*to;
        int r = sendto(_fd, (const char*)data, len, 0, *to, to.len);
        if (r < 0){
          // sendto failed to send with an error
          auto e = err();
          printf("sendto failed to send with error: %d", e);
        }
        else if (r < len)
        {
          // sendto didn't send all the bytes
        }
        assert(r == len);
        return r;
      }
    };

    Socket* createSocket(SocketType type)
    {
      return new Socket(int(type));
    }

  }


}