#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

#include <asio.hpp>

using asio::ip::tcp;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 private:
  static const size_t kBufCapacity = 1024;

 public:
  explicit TcpConnection(tcp::socket socket) : socket_(std::move(socket)) {
  }

  void Start() {
    ReadChunk();
  }

 private:
  void ReadChunk() {
    auto self = shared_from_this();
    socket_.async_read_some(
        asio::buffer(buf_, kBufCapacity),
        [this, self](std::error_code error_code, std::size_t bytes_read) {
          (void)self;
          if (!error_code) {
            WriteChunk(bytes_read);
          }
        });
  }

  void WriteChunk(std::size_t bytes) {
    auto self = shared_from_this();
    asio::async_write(
        socket_, asio::buffer(buf_, bytes),
        [this, self](std::error_code /*error_code*/, std::size_t /*bytes*/) {
          // Write completed!
        });
  }

 private:
  tcp::socket socket_;
  char buf_[kBufCapacity];
};

class EchoServer {
 public:
  EchoServer(asio::io_context& io_context, unsigned short port)
      : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    AcceptClient();
  }

 private:
  void AcceptClient() {
    acceptor_.async_accept(
        [this](std::error_code error_code, tcp::socket client_socket) {
          if (!error_code) {
            std::make_shared<TcpConnection>(std::move(client_socket))->Start();
          }
        });
  }

 private:
  tcp::acceptor acceptor_;
};
