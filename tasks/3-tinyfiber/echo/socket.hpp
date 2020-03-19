#pragma once

#include <tinysupport/result.hpp>

#include <asio.hpp>

namespace tinyfiber {
namespace tcp {

using MutableBuffer = asio::mutable_buffer;
using ConstBuffer = asio::const_buffer;

class Socket {
 public:
  static Result<Socket> ConnectTo(const std::string& host, uint16_t port);
  static Result<Socket> ConnectToLocal(uint16_t port);

  // Will block until
  // * supplied buffer is full
  // * end of stream reached
  // * an error occurred
  // Returns number of bytes received
  // Bytes received < buffer size means end of stream
  Result<size_t> Read(MutableBuffer buffer);

  // Will block until
  // * at least one byte is read
  // * end of stream reached
  // * an error occurred
  // Returns number of bytes received or 0 if end of stream reached
  Result<size_t> ReadSome(MutableBuffer buffer);

  // Will block until
  // * all of the bytes in the buffer are sent or
  // * an error occurred
  Status Write(ConstBuffer buffer);

  // Shutting down the send side of the socket
  Status ShutdownWrite();

 private:
  // Use asio::ip::tcp::socket
};

class Acceptor {
 public:
  Acceptor();

  Status Listen(uint16_t port);

  // Returns port number
  Result<uint16_t> ListenAvailablePort();

  uint16_t GetPort() const;

  Result<Socket> Accept();

 private:
  // Use asio::ip::tcp::acceptor
};

}  // namespace tcp
}  // namespace tinyfiber
