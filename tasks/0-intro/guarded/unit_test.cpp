#include <twist/test_framework/test_framework.hpp>
#include <twist/strand/stdlike.hpp>
#include <twist/strand/test.hpp>

#include <guarded.hpp>

#include <chrono>
#include <set>
#include <string>
#include <vector>

TEST_SUITE(Guarded) {
  SIMPLE_T_TEST(Vector) {
    solutions::Guarded<std::vector<int>> vector;

    ASSERT_TRUE(vector->empty());

    vector->push_back(42);
    ASSERT_EQ(vector->front(), 42);
    ASSERT_EQ(vector->size(), 1);

    vector->push_back(99);
    ASSERT_EQ(vector->size(), 2);
  }

  SIMPLE_T_TEST(Set) {
    solutions::Guarded<std::set<std::string>> strings;

    strings->insert("Hello");
    strings->insert("World");
    strings->insert("!");

    ASSERT_EQ(strings->size(), 3);
  }

  SIMPLE_T_TEST(Counter) {
    struct Counter {
      int value = 0;

      void SlowIncrement() {
        using namespace std;
        int old = value;
        twist::strand::this_thread::sleep_for(50ms);
        value = old + 1;
      }
    };

    solutions::Guarded<Counter> counter;

    auto call_increment = [&]() { counter->SlowIncrement(); };

   twist::strand::thread thread1(call_increment);
   twist::strand:: thread thread2(call_increment);
    thread1.join();
    thread2.join();

    ASSERT_EQ(counter->value, 2);
  }
}

RUN_ALL_TESTS()
