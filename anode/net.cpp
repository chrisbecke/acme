#include "net.hpp"
#pragma comment(lib,"ws2_32.lib")
#include <assert.h>
#include <stdio.h>

using namespace anode;
using namespace net;



namespace anode {
  namespace net {

    static Address sock_address(sockaddr* name)
    {
      Address result;

      string_buffer address(64); // there is a constant for this value
      int family = name->sa_family;
      result.family = Family(family);
      void* addr = nullptr;
      if (name->sa_family == AF_INET){
        sockaddr_in* in4 = (sockaddr_in*)name;
        addr = &in4->sin_addr;
        result.port = ntohs(in4->sin_port);
      }
      else if (name->sa_family == AF_INET6){
        sockaddr_in6* in6 = (sockaddr_in6*)name;
        result.port = ntohs(in6->sin6_port);
        addr = &in6->sin6_addr;
      }
      inet_ntop(family, addr, address, address.size());
      result.address = address;
      return result;
    }

    static Address sock_address(SOCKET fd)
    {
      Address result;

      sockaddr name;
      int len = sizeof(name);
      int r = getsockname(fd, &name, &len);
      assert(r == 0);

      return sock_address(&name);
    }


    class WindowsSocketContext
    {
#ifdef _WIN32
    public:
      WindowsSocketContext(){
        WSADATA wsad;
        WSAStartup(0x0101, &wsad);
      }

      ~WindowsSocketContext(){
        WSACleanup();
      }

      WindowsSocketContext& getInstance(){
        static WindowsSocketContext ctx;
        return ctx;
      }
#endif
    };

    Address Socket::address() const
    {
      return sock_address(_socket);
    }

    Socket& Socket::connect(int port, const char* host,connectListener onConnect)
    {
      addrinfo *result = NULL;
      addrinfo *ptr = NULL;
      addrinfo hints = { 0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0 };
      char sport[8];
      sprintf(sport, "%d", port);
      int r = getaddrinfo(host, sport, &hints, &result);
      if (r)
        return *this;
      for (addrinfo* ptr = result; ptr; ptr = ptr->ai_next){
        SOCKET newSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        // should put the socket into non blocking mode here before calling connect.
        r = ::connect(newSocket, ptr->ai_addr, ptr->ai_addrlen);
        if (r){
          closesocket(newSocket);
          newSocket = INVALID_SOCKET;
          continue;
        }
        _socket = newSocket;
        break;
      }
      freeaddrinfo(result);
      subscribe(true);
      return *this;
    }

    void Socket::destroy()
    {
      if (!destroyed){
        destroyed = true;
        _close();
      }
      if (_socket != INVALID_SOCKET)
        shutdown(_socket, SD_BOTH);
      subscribe(false);
    }



    void Socket::end(const char* chunk, int len){
      if (chunk)
        write(chunk, len);
      shutdown(_socket, SD_SEND);
    }

    void Socket::subscribe(bool sub)
    {
      if (_socket == INVALID_SOCKET)
        return;

      auto app = SocketServerRunLoop::getInstance();
      if (sub)
      {
        app->ReadReady[_socket] = {
          this,
          [=](){
            char buf[2048];
            int len = sizeof(buf);
            int received = recv(_socket, buf, len, 0);
            if (received <= 0){
              destroy();
            }
            else {
              if (_data)
                _data(buf, len);
            }

          }
        };

      }
      else
      {
        app->ReadReady -= _socket;
        app->WriteReady -= _socket;
        app->ErrorReady -= _socket;
      }
    }

    bool Socket::write(const char* chunk, int len)
    {
      assert(len > 0);
      assert(_socket != INVALID_SOCKET);
      int cbSent = send(_socket, chunk, len, 0);
      if (cbSent == 0){
        _close();
      }
      else if (cbSent < 0)
      {
        destroy();
      }

      assert(cbSent == len);
      return cbSent == len;
    }

    Address Server::address() const
    {
      return sock_address(_socket);
    }

    Server& Server::listen(int port, const char* host, int backlog, function<void()> callback){
      onListening+= callback;
      int r;
      struct addrinfo* result;
      struct addrinfo hints = { AI_PASSIVE, AF_INET, SOCK_STREAM, IPPROTO_TCP, 0 };
      char saddr[8];
      sprintf(saddr, "%d", port);
      r = getaddrinfo(host, saddr, &hints, &result);
      if (r)
      {
        Error err(r);
        onError(&err);
      }
      auto fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
      assert(fd != INVALID_SOCKET);
      r = bind(fd, result->ai_addr, result->ai_addrlen);
      assert(!r);
      freeaddrinfo(result);
      _socket = fd;
      if (onListening)
        onListening();
      r = ::listen(fd, backlog>0?backlog:SOMAXCONN);
      assert(!r);

      auto app = SocketServerRunLoop::getInstance();

      app->ReadReady[fd] = {
        this,
        [=](){
          struct sockaddr addr = { 0 };
          int len = sizeof(addr);
          SOCKET newSocket = ::accept(fd, &addr, &len);
          assert(newSocket != INVALID_SOCKET);

          auto socket = new Socket(newSocket);
          socket->remoteAddress = sock_address(&addr);
          if (onConnection)
            onConnection(socket);
        } 
      };

      return *this;
    }

    SocketServerRunLoop::SocketServerRunLoop()
    {
      WSADATA wsa;
      int result = WSAStartup(0x0202, &wsa);
      assert(!result);
    }

    SocketServerRunLoop::~SocketServerRunLoop()
    {
      WSACleanup();
    }

    bool SocketServerRunLoop::setfds(fd_set*& fds, const map<int, Event>& ReadyList)
    {
      if (ReadyList){
        if (!fds)
          fds = new fd_set;
        FD_ZERO(fds);
        for (auto item : ReadyList){
          FD_SET(item.key, fds);
        }
        return true;
      }
      else if (fds){
        delete fds;
        fds = nullptr;
      }
      return false;
    }

    bool SocketServerRunLoop::notify(fd_set* fds, const map<int, Event>& ReadyList)
    {
      if (!ReadyList || !fds)
        return false;

      list<function<void()>> toNotify;

      bool didNotify = false;
      for (auto item : ReadyList){
        if (FD_ISSET(item.key, fds)){
          toNotify += item.value.callback;
          // call the notify lambda
          didNotify = true;
        }
      }

      for (auto callback : toNotify)
        callback();

      return didNotify;
    }

    int SocketServerRunLoop::Run()
    {
      fd_set* readfds = nullptr;
      fd_set* writefds = nullptr;
      fd_set* exceptfds = nullptr;

      for (;;)
      {
        bool isWork = false;
        isWork |= setfds(readfds, ReadReady);
        isWork |= setfds(writefds, WriteReady);
        isWork |= setfds(exceptfds, ErrorReady);

        if (!isWork)
          break;

        int result = select(0, readfds, writefds, exceptfds, nullptr);

        notify(readfds, ReadReady);
        notify(writefds, WriteReady);
        notify(exceptfds, ErrorReady);
      }

      return 0;
    }



  }
}

Socket* anode::net::connect(int port, const char* hostname, connectListener onConnect)
{
  Socket* socket = new Socket();
  socket->connect(port, hostname, onConnect);
  return socket;
}

Server* anode::net::createServer(connectionListener listener)
{
  auto server = new Server();
  server->onConnection += listener;
  return server;
}

