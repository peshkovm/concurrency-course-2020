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
  return Ok<size_t>(0);
}

Result<size_t> Socket::Read(MutableBuffer buffer) {
  // Not implemented
  return Ok<size_t>(0);
}

Status Socket::Write(ConstBuffer buffer) {
  // Not implemented
  return Ok();
}

Status Socket::ShutdownWrite() {
  // Not implemented
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
}

Result<Socket> Acceptor::Accept() {
  // Not implemented
}

uint16_t Acceptor::GetPort() const {
  // Not implemented
}

}  // namespace tcp
}  // namespace tinyfiber
