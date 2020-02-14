#pragma once

#include <cstdlib>
#include <string>
#include <utility>

#include <asio.hpp>

class BlockingTcpClient {
 public:
  BlockingTcpClient(const std::string& host, uint16_t port)
      : resolver_(io_context_), socket_(io_context_) {
    ConnectTo(host, port);
  }

  void Send(const std::string& data) {
    asio::write(socket_, asio::buffer(data.data(), data.size()));
  }

  std::string Recv(size_t bytes) {
    std::string reply;
    reply.resize(bytes);
    size_t bytes_read =
        asio::read(socket_, asio::buffer(reply.data(), reply.size()));
    reply.resize(bytes_read);
    return reply;
  }

 private:
  void ConnectTo(const std::string& host, uint16_t port) {
    auto addr = resolver_.resolve(host, std::to_string(port));
    asio::connect(socket_, addr);
  }

 private:
  asio::io_context io_context_;
  asio::ip::tcp::resolver resolver_;
  asio::ip::tcp::socket socket_;
};
