#include <chrono>
#include <list>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <asio.hpp>

#include <twist/test_framework/test_framework.hpp>

#include "server.hpp"
#include "client.hpp"

using namespace std::chrono_literals;

static const std::string kServerHost = "localhost";
static const uint16_t kServerPort = 51423;

void LaunchEchoServer() {
  auto runner = []() {
    asio::io_context server_io;
    EchoServer server(server_io, kServerPort);
    server_io.run();
  };
  // Serve forever in background thread
  std::thread(runner).detach();
}

std::string Echo(BlockingTcpClient& client, const std::string& data) {
  client.Send(data);
  return client.Recv(data.length());
}

TEST_SUITE(EchoServer) {
  SIMPLE_TEST(ServerLaunched) {
  }

  SIMPLE_TEST(Hello) {
    BlockingTcpClient client(kServerHost, kServerPort);
    ASSERT_EQ(Echo(client, "Hello"), "Hello");
  }

  SIMPLE_TEST(HelloWorld) {
    BlockingTcpClient client(kServerHost, kServerPort);

    client.Send("Hello");
    std::this_thread::sleep_for(1s);
    client.Send(", World");
    ASSERT_EQ(client.Recv(12), "Hello, World");

    // No extra data from server
    ASSERT_EQ(Echo(client, "!"), "!");
  }

  SIMPLE_TEST(TwoClients) {
    BlockingTcpClient finn(kServerHost, kServerPort);
    BlockingTcpClient jake(kServerHost, kServerPort);

    finn.Send("Hi, Jake!");
    jake.Send("Hi, Finn!");

    std::this_thread::sleep_for(1s);

    ASSERT_EQ(finn.Recv(9), "Hi, Jake!");
    ASSERT_EQ(jake.Recv(9), "Hi, Finn!");
  }

  SIMPLE_TEST(ConcurrentClients) {
    static const size_t kClients = 17;

    std::list<BlockingTcpClient> clients;
    for (size_t i = 0; i < kClients; ++i) {
      clients.emplace_back(kServerHost, kServerPort);
    }

    static const std::string kMessage = "Merry Cristmas!";

    for (auto& client : clients) {
      client.Send(kMessage);
    }

    for (auto& client : clients) {
      ASSERT_EQ(client.Recv(kMessage.length()), kMessage);
    }
  }

  class MessageGenerator {
   public:
    MessageGenerator(size_t seed)
      : twister_(seed) {
    }

    std::string Next() {
      message_ = message_ + std::to_string(twister_()) + message_;
      return message_;
    }

   private:
    std::mt19937 twister_;
    std::string message_;
  };

  SIMPLE_TEST(GrowingMessages) {
    static const size_t kIterations = 17;

    auto run_client = [&](size_t thread_index) {
      MessageGenerator generator(thread_index);

      for (size_t i = 0; i < kIterations; ++i) {
        auto message = generator.Next();
        BlockingTcpClient client(kServerHost, kServerPort);
        ASSERT_EQ(Echo(client, message), message);
        std::this_thread::sleep_for(100ms);
      }
    };

    static const size_t kThreads = 7;

    std::vector<std::thread> threads;
    for (size_t i = 0; i < kThreads; ++i) {
      threads.emplace_back(run_client, i);
    }

    for (auto& thread : threads) {
      thread.join();
    }
  }
}

int main() {
  LaunchEchoServer();
  RunTests(ListAllTests());
}
