#include "socket.hpp"

namespace tinyfiber {
namespace tcp {

using namespace make_result;

Result<Socket> Socket::ConnectTo(const std::string& host, uint16_t port) {
  // Not implemented
}

Result<Socket> Socket::ConnectToLocal(uint16_t port) {
  // Not implemented
}

Result<size_t> Socket::ReadSome(MutableBuffer buffer) {
  // Not implemented
  // Use 'async_read_some` method of ip::tcp::socket
  return Ok<size_t>(0);
}

Result<size_t> Socket::Read(MutableBuffer buffer) {
  // Not implemented
  return Ok<size_t>(0);
}

Status Socket::Write(ConstBuffer buffer) {
  // Not implemented
  // Use `asio::async_write` function
  return Ok();
}

Status Socket::ShutdownWrite() {
  // Not implemented
  // Use `shutdown` method of ip::tcp::socket
  return Ok();
}

Acceptor::Acceptor() {
  // Not implemented
}

Status Acceptor::Listen(uint16_t port) {
  // Not implemented
}

Result<uint16_t> Acceptor::ListenAvailablePort() {
  // Not implemented
  // Use Listen(0) to automatically choose available port
}

Result<Socket> Acceptor::Accept() {
  // Not implemented
}

uint16_t Acceptor::GetPort() const {
  // Not implemented
}

}  // namespace tcp
}  // namespace tinyfiber
