#pragma once

#include "api.hpp"
#include "socket.hpp"

namespace echo {

class EchoServer {
 public:
  EchoServer(uint16_t port);
  void ServeForever();
};

}  // namespace echo
