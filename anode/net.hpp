#pragma once
#include "core/function.hpp"
#include "core/collections.hpp"
#include "core/managed.hpp"
#include "core/object.hpp"
#include "core/runtime.hpp"
#include "core/string.hpp"

// I will probably regret this foray into winsock2 and it will break anodes portability to other platforms.
#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

namespace anode {
  namespace net {

    class Error {
    public:
      Error(int c){
        code = ErrorCode(c);
      }
      enum ErrorCode {
      NOTINITIALISED = 10093,
      } code;
    };

    enum Family {
      inetv4 = 2,
      inetv6 = 4
    };

    struct Address {
      Family family;
      string address;
      int port;
    };

    class Socket;
    class Server;

    typedef function<void(Socket*)> connectionListener;
    typedef function<void()> connectListener;

    Socket* connect(int port, const char* hostname=nullptr, connectListener=nullptr);

    Server* createServer(connectionListener);

    bool isIP(const char*);
    bool isIPv4(const char*);
    bool isIPv6(const char*);

    // custom
    // returns true if there are active net objects to dispatch.
    bool dispatch(int timeout);

    class Socket : public Object {
    protected:
      SOCKET _socket;
      unsigned long _ref;
      Event<void()> _close;
      Event<void(const char* chunk, int len)> _data;
      Event<void()> _connectListeners;
    public:
      Address remoteAddress;
      bool connecting;
      bool destroyed;

    public:
      Socket(SOCKET fd=INVALID_SOCKET) :_socket(fd),_ref(false){
        subscribe(true);
        destroyed = false;
        connecting = false;
      }
      Socket(const Socket& other) = delete;
      Socket(Socket&& other){
        _socket = other._socket;
        _ref = other._ref;
        other._socket = INVALID_SOCKET;
        other._ref = 0;
      }
      ~Socket(){
        // now this is unexpected
        assert(!_ref);
        if (_socket != INVALID_SOCKET)
          closesocket(_socket);
        subscribe(false);
        _socket = INVALID_SOCKET;
      }
      SOCKET operator*(){
        _socket;
      }
      Socket& operator=(const Socket& other) = delete;
      Socket& operator=(Socket&& other) = delete;

      void onConnect(function<void()> callback){
        _connectListeners += callback;
      }

      void onData(function<void(const char*, int)> callback){
        _data += callback;
        autoref(0);
      }
      void onClose(function<void()> callback){
        _close += callback;
        autoref(0);
      }

      operator bool() const{
        return _socket != INVALID_SOCKET;
      }

      Address address() const;
      Socket& connect(int port, const char* host=nullptr,connectListener onConnect=nullptr);
      Socket& connect(int port, connectListener onConnect = nullptr){
        return connect(port, nullptr, onConnect);
      }
      void destroy();
      void end(const char* chunk = nullptr, int len = 0);
      // localAddress
      // localPort
      // pause
      // 
      Socket& ref(){
        autoref(1);
        return *this;
      }
      // remoteAddress
      // remoteFamily
      // remotePort
      // resume
      // setEncoding
      // setKeepAlive
      // setNoDelay
      // setTimeout
      Socket& unref(){
        autoref(2);
        return *this;
      }
      bool write(const char* chunk, int len);
    protected:
      void autoref(int refAction){
        auto newRef = (_close ? 4 : 0) | (_data ? 2 : 0) | (refAction ? refAction & 1 : (_ref & 1));
        // check the state change
        if (_ref && !newRef)
          release();
        else if (newRef && !_ref)
          retain();
        _ref = newRef;
      }
      void subscribe(bool sub = true);
    };

    typedef Reference<Socket> SocketRef;

    class Server : Object {
      SOCKET _socket;
    public:
      Event<void(Socket*)> onConnection;
      Event<void()> onListening;
      Event<void()> onClose;
      Event<void(Error*)> onError;

      struct address {
        int port;
        string hostname;

      };
    public:
      Server(SOCKET fd = INVALID_SOCKET) : _socket(fd){}
      Server(const Server& other) = delete;
      Server(Server&& other){
        _socket = other._socket;
        other._socket = INVALID_SOCKET;
      }
      ~Server(){
        if (_socket != INVALID_SOCKET)
          closesocket(_socket);
      }
      Server& operator=(const Server& other) = delete;
      Server& operator=(Server&& other) = delete;

      Server& listen(int port, const char* host, int backlog=-1,function<void()> callback=nullptr);
      Server& listen(int port, const char* host, function<void()> callback = nullptr){
        return listen(port, host, -1, callback);
      }
      Server& listen(int port, function<void()> callback = nullptr){
        return listen(port, nullptr, -1, callback);
      }
      Server& listen(function<void()> callback = nullptr){
        return listen(0, nullptr, -1, callback);
      }
      Address address() const;
      void close(function<void()> callback = nullptr);
    };

    class SocketServerRunLoop : public runtime::Application
    {
      typedef Application super;
    public:
      struct Event {
        ObjectRef socket;
        function<void()> callback;
      };
      map<int, Event> ReadReady;
      map<int, Event> WriteReady;
      map<int, Event> ErrorReady;
    public:
      SocketServerRunLoop();
      ~SocketServerRunLoop();

      int Run();

      static SocketServerRunLoop* getInstance(){ return (SocketServerRunLoop*)super::getInstance(); }

    protected:

      static bool setfds(fd_set*& fds, const map<int, Event>& ReadyList);
      static bool notify(fd_set* fds, const map<int, Event>& ReadyList);


    };

    template<typename lambda_t>
    int SocketServerMain(lambda_t lambda)
    {
      SocketServerRunLoop App;

      lambda();

      return App.Run();
    }

  }
}
