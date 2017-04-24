#pragma once

#include <assert.h>
#include "core/collections.hpp"
#include "core/function.hpp"
#include "net.hpp"

#ifdef REFACTORING

namespace anode {

  namespace http {
    using TcpServer = anode::net::Server;
    using anode::net::Socket;

    class ServerRequest{

    };
    class ServerResponse{

    };


    class Server {
    public:
      TcpServer serverSocket;

    public:
      void listen(int port, const char* hostname, int backlog, function<void(ServerRequest,ServerResponse)> callback){
        serverSocket.listen(8080, [=](Socket socket){
          // connections arrive here.
          ServerRequest request;
          ServerResponse response;

          socket.data([](){
          });

//          socket.data([](){});

          callback(request, response);
        });
      }
      void listen(int port, function<void(ServerRequest, ServerResponse)> callback){
        listen(port, nullptr, SOMAXCONN, callback);
      }
    };


    class http {
    public:
      static Server createServer(function<void(ServerRequest, ServerResponse)> notify){
        Server server;
        server.listen(8080, notify);

        return server;
      }
    };
  }

}
#endif