#include <algorithm>
#include <chrono>
#include <list>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <sstream>

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

  class MessageBuffer {
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

  SIMPLE_TEST(DelayedReads) {
    static const size_t kBytesToSend = 16 * 1024 * 1024;
    static const size_t kChunkLimit = 1024 * 1024;

    BlockingTcpClient client(kServerHost, kServerPort);
    MessageBuffer sent, received;

    auto write = [&]() {
      DataGenerator generator(kBytesToSend);

      char write_buf[kChunkLimit];
      while (generator.HasMore()) {
        const size_t chunk_size = generator.NextChunk(write_buf, kChunkLimit);
        client.Send(write_buf, chunk_size);
        sent.Append(write_buf, chunk_size);
      }
    };

    auto read = [&]() {
      std::this_thread::sleep_for(3s);

      char read_buf[kChunkLimit];
      while (received.Size() < kBytesToSend) {
        size_t bytes_read = client.Recv(read_buf, kChunkLimit);
        received.Append(read_buf, bytes_read);
      }
    };

    std::thread writer(write);
    std::thread reader(read);
    writer.join();
    reader.join();

    ASSERT_EQ(sent.Size(), kBytesToSend);
    ASSERT_EQ(sent.ToString(), received.ToString());
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
