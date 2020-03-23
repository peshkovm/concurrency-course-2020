#include "api.hpp"
#include "scheduler.hpp"
#include "socket.hpp"
#include "echo.hpp"

#include <twist/test_framework/test_framework.hpp>

#include <twist/support/random.hpp>
#include <twist/support/time.hpp>
#include <twist/support/string_builder.hpp>

#include <cmath>
#include <ctime>
#include <iostream>
#include <functional>
#include <random>
#include <set>
#include <thread>

// Test utilities

//////////////////////////////////////////////////////////////////////

class DataGenerator {
 public:
  DataGenerator(size_t bytes)
      : bytes_left_(bytes) {
  }

  bool HasMore() const {
    return bytes_left_ > 0;
  }

  size_t NextChunk(char* buf, size_t limit) {
    size_t bytes = std::min(bytes_left_, limit);
    GenerateTo(buf, bytes);
    bytes_left_ -= bytes;
    return bytes;
  }
 private:
  void GenerateTo(char* buf, size_t bytes) {
    for (size_t i = 0; i < bytes; ++i) {
      buf[i] = 'A' + twister_() % 26;
    }
  }

 private:
  size_t bytes_left_;
  std::mt19937 twister_{42};
};

//////////////////////////////////////////////////////////////////////

class GrowingOutBuffer {
 public:
  size_t Size() const {
    return size_;
  }

  void Append(char* buf, size_t bytes) {
    buf_.write(buf, bytes);
    size_ += bytes;
  }

  std::string ToString() const {
    return buf_.str();
  }

 private:
  std::ostringstream buf_;
  size_t size_ = 0;
};

//////////////////////////////////////////////////////////////////////

using namespace tinyfiber;
using namespace tinyfiber::tcp;

//////////////////////////////////////////////////////////////////////

// Read variations

static Result<std::string> ReadRequired(Socket& socket, size_t bytes) {
  std::string data;
  data.resize(bytes);
  auto bytes_read = socket.Read({data.data(), data.size()});
  if (!bytes_read) {
    return make_result::PropagateError(bytes_read);
  }
  data.resize(*bytes_read);
  return make_result::Ok(data);
}

static Result<std::string> ReadAll(Socket& socket) {
  GrowingOutBuffer read;

  static const size_t kBufSize = 1237;
  char buf[kBufSize];

  while (true) {
    auto bytes_read = socket.Read(asio::buffer(buf, kBufSize));
    if (!bytes_read) {
      return make_result::PropagateError(bytes_read);
    }
    if (*bytes_read > 0) {
      read.Append(buf, *bytes_read);
    }
    if (*bytes_read < kBufSize) {
      return make_result::Ok(read.ToString());
    }
  }
}

//////////////////////////////////////////////////////////////////////

class SimpleTcpClient {
 public:
  SimpleTcpClient(std::string host, uint16_t port)
    : socket_(Socket::ConnectTo(host, port)) {
  }

  Result<void> Write(const std::string& data) {
    return socket_.Write(asio::buffer(data));
  }

  Result<std::string> ReadRequired(const size_t bytes) {
    return ::ReadRequired(socket_, bytes);
  }

  Result<std::string> ReadAll() {
    return ::ReadAll(socket_);
  }

  Result<void> ShutdownWrite() {
    return socket_.ShutdownWrite();
  }

 private:
  Socket socket_;
};

//////////////////////////////////////////////////////////////////////

void RunFiberTest(FiberRoutine test) {
  bool done{false};
  RunScheduler([&]() {
    test();
    done = true;
  });
  ASSERT_TRUE_M(done, "Init fiber not completed");
}

//////////////////////////////////////////////////////////////////////

// Sockets

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Sockets) {
  SIMPLE_TEST(ChooseAvailablePort) {
    RunFiberTest([]() {
      Acceptor acceptor_1;
      acceptor_1.Listen(0).ExpectOk();

      Acceptor acceptor_2;
      acceptor_2.Listen(0).ExpectOk();

      std::cout << "Available ports: "
        << acceptor_1.GetPort() << ", "
        << acceptor_2.GetPort() << std::endl;

      ASSERT_NE(acceptor_1.GetPort(), acceptor_2.GetPort());
    });
  }

  SIMPLE_TEST(AddressAlreadyInUse) {
    RunFiberTest([]() {
      Acceptor acceptor_1;
      acceptor_1.ListenAvailablePort().ThrowIfError();

      Acceptor acceptor_2;
      auto status = acceptor_2.Listen(acceptor_1.GetPort());
      ASSERT_FALSE(status.IsOk());
    });
  }

  SIMPLE_TEST(FailToConnect) {
    auto test = []() {
      Acceptor acceptor;
      uint16_t port = acceptor.ListenAvailablePort();

      auto connect = [port]() {
        auto socket = Socket::ConnectToLocal(port);
        ASSERT_TRUE(socket.HasError());
        if (!socket) {
          std::cout << "Cannot connect ot port " << port << ": "
                    << socket.Error().message() << std::endl;
        }
      };
      Spawn(connect);

      //Socket client_socket = acceptor.Accept();
    };

    RunFiberTest(test);
  }

  // Broke blocking accept
  SIMPLE_TEST(Accept) {
    auto test = []() {
      Acceptor acceptor;
      uint16_t port = acceptor.ListenAvailablePort();

      auto connect = [port]() {
        Socket socket = Socket::ConnectToLocal(port);
      };
      Spawn(connect);

      Socket client_socket = acceptor.Accept();
    };

    RunFiberTest(test);
  }

  // Broke blocking connect
  SIMPLE_TEST(Connect) {
    auto test = []() {
      Acceptor acceptor;
      uint16_t port = acceptor.ListenAvailablePort();

      auto accept = [&acceptor]() {
        Socket socket = acceptor.Accept();
      };
      Spawn(accept);

      Socket socket = Socket::ConnectToLocal(port);
    };

    RunFiberTest(test);
  }

  SIMPLE_TEST(Hello) {
    auto test = []() {;
      static const std::string kHelloMessage = "Hello, World!";

      Acceptor acceptor;
      uint16_t port = acceptor.ListenAvailablePort();

      auto client = [port]() {
        Socket socket = Socket::ConnectToLocal(port);
        socket.Write(asio::buffer(kHelloMessage)).ExpectOk();
      };
      Spawn(client);

      Socket socket = acceptor.Accept();
      std::string message = ReadRequired(socket, kHelloMessage.length());
      std::cout << "Message sent: '" << kHelloMessage << "'" << std::endl;
      std::cout << "Message received: " << message << "'" << std::endl;
      ASSERT_EQ(message, kHelloMessage);
    };

    RunFiberTest(test);
  }

  SIMPLE_TEST(ReadAll) {
    auto test = []() {;
      static const std::string kHelloMessage = "Hello, World!";

      Acceptor acceptor;
      uint16_t port = acceptor.ListenAvailablePort();

      auto client = [port]() {
        Socket socket = Socket::ConnectToLocal(port);
        socket.Write(asio::buffer(kHelloMessage)).ExpectOk();
        socket.ShutdownWrite().ExpectOk();
      };
      Spawn(client);

      Socket socket = acceptor.Accept();

      std::string message = ReadAll(socket);
      ASSERT_EQ(message, kHelloMessage);
    };

    RunFiberTest(test);
  }

  SIMPLE_TEST(Shutdown) {
    auto test = []() {;
      static const std::string kHelloMessage = "Hi, Neo!";
      static const std::string kReplyMessage = "Hi, Morpheus!";

      Acceptor acceptor;
      uint16_t port = acceptor.ListenAvailablePort();

      auto client = [port]() {
        Socket socket = Socket::ConnectToLocal(port);
        socket.Write(asio::buffer(kHelloMessage)).ExpectOk();

        socket.ShutdownWrite().ExpectOk();

        std::string reply = ReadAll(socket);
        ASSERT_EQ(reply, kReplyMessage);
      };
      Spawn(client);

      Socket client_socket = acceptor.Accept();
      std::string message = ReadAll(client_socket);
      ASSERT_EQ(message, kHelloMessage);

      client_socket.Write(asio::buffer(kReplyMessage)).ExpectOk();
      client_socket.ShutdownWrite().ExpectOk();
    };

    RunFiberTest(test);
  }

  SIMPLE_TEST(SocketStreamReads) {
    auto test = []() {;
      static const size_t kStreamLength = 16 * 1024 * 1024;
      static const size_t kWriteBufSize = 4113;
      static const size_t kReadBufSize = 3517;

      Acceptor acceptor;
      uint16_t port = acceptor.ListenAvailablePort();

      DataGenerator generator(kStreamLength);
      GrowingOutBuffer sent;
      GrowingOutBuffer received;

      auto sender = [port, &generator, &sent]() {
        Socket socket = Socket::ConnectToLocal(port);

        char write_buf[kWriteBufSize];
        while (generator.HasMore()) {
          size_t buf_size = generator.NextChunk(write_buf, kWriteBufSize);
          socket.Write(asio::buffer(write_buf, buf_size)).ExpectOk();
          sent.Append(write_buf, buf_size);
        }
        socket.ShutdownWrite().ExpectOk();
      };
      Spawn(sender);

      Socket socket = acceptor.Accept();

      char read_buf[kReadBufSize];
      while (true) {
        size_t bytes_read = socket.ReadSome(asio::buffer(read_buf, kReadBufSize));
        if (bytes_read > 0) {
          received.Append(read_buf, bytes_read);
        } else {
          break;
        }
      }

      ASSERT_EQ(sent.ToString(), received.ToString());
    };

    RunFiberTest(test);
  }

  SIMPLE_TEST(SocketHugeRead) {
    auto test = []() {;
      static const std::string kHelloMessage = "Hello, World!";
      static const size_t kStreamLength = 8 * 1024 * 1024;
      static const size_t kChunkSize = 4096;

      Acceptor acceptor;
      uint16_t port = acceptor.ListenAvailablePort();

      DataGenerator generator(kStreamLength);
      GrowingOutBuffer sent;

      auto sender = [port, &generator, &sent]() {
        Socket socket = Socket::ConnectToLocal(port);

        char chunk[kChunkSize];
        while (generator.HasMore()) {
          size_t chunk_size = generator.NextChunk(chunk, kChunkSize);
          socket.Write(asio::buffer(chunk, chunk_size)).ExpectOk();
          sent.Append(chunk, chunk_size);
        }
      };
      Spawn(sender);

      Socket socket = acceptor.Accept();

      std::string read_buf;
      read_buf.resize(kStreamLength);

      size_t bytes_read = socket.Read(asio::buffer(read_buf.data(), read_buf.size()));
      ASSERT_EQ(bytes_read, kStreamLength);
      ASSERT_EQ(read_buf, sent.ToString());
    };

    RunFiberTest(test);
  }

  SIMPLE_TEST(HttpBin) {
    auto test = []() {
      Socket socket = Socket::ConnectTo("httpbin.org", 80);

      static const std::string_view kCRLF = "\r\n";

      std::string request = twist::StringBuilder()
          << "GET /status/200 HTTP/1.0" << kCRLF
          << "Host: httpbin.org" << kCRLF
          << kCRLF;

      socket.Write(asio::buffer(request)).ExpectOk();
      socket.ShutdownWrite().ExpectOk();

      std::string response = ReadAll(socket);

      std::cout << "Response from httpbin: " << response << std::endl;
    };

    RunFiberTest(test);
  }
}

//////////////////////////////////////////////////////////////////////

// Echo server

//////////////////////////////////////////////////////////////////////

static uint16_t GetEchoServerPort() {
  // https://xkcd.com/221/
  static const uint16_t kPort = 31532;

  return kPort;
}

void StartEchoServer() {
  std::thread([]() {
    RunScheduler([]() {
      echo::EchoServer server{GetEchoServerPort()};
      server.ServeForever();
    });
  }).detach();
}

SimpleTcpClient MakeEchoClient() {
  return {"localhost", GetEchoServerPort()};
}

TEST_SUITE(EchoServer) {
  SIMPLE_TEST(HelloWorld) {
    RunFiberTest([]() {
      static const std::string kMessage = "Hello, World!";

      Socket socket = Socket::ConnectToLocal(
          GetEchoServerPort());

      socket.Write(asio::buffer(kMessage)).ExpectOk();
      std::string response = ReadRequired(socket, kMessage.length());
      ASSERT_EQ(response, kMessage);

      socket.Write(asio::buffer("!")).ExpectOk();
      std::string terminator = ReadRequired(socket, 1);

      socket.ShutdownWrite().ExpectOk();

      ASSERT_EQ(terminator, "!");
    });
  }

  SIMPLE_TEST(ShutdownWrite) {
    auto test = []() {
      static const std::string kMessage = "Test shutdown";

      auto client = MakeEchoClient();

      client.Write(kMessage).ExpectOk();
      client.ShutdownWrite().ExpectOk();

      ASSERT_EQ(client.ReadAll().Value(), kMessage);
    };

    RunFiberTest(test);
  }

  SIMPLE_TEST(TwoClients) {
    auto test = []() {
      auto finn = MakeEchoClient();
      auto jake = MakeEchoClient();

      finn.Write("Hi, Jake!").ExpectOk();
      jake.Write("Hi, Finn!").ExpectOk();

      ASSERT_EQ(finn.ReadRequired(9).Value(), "Hi, Jake!");
      ASSERT_EQ(jake.ReadRequired(9).Value(), "Hi, Finn!");
    };

    RunFiberTest(test);
  }
}

//////////////////////////////////////////////////////////////////////

int main() {
  StartEchoServer();
  RunTests(ListAllTests());
}
