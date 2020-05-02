#include <tinyfutures/executors/inline.hpp>
#include <tinyfutures/executors/helpers.hpp>

namespace tiny::executors {

class InlineExecutor : public IExecutor {
 public:
  void Execute(Task&& task) override {
    SafelyExecuteHere(task);
  }

  void WorkCreated() override {
  }

  void WorkCompleted() override {
  }
};

class Instance {
 public:
  Instance()
    : e_(std::make_shared<InlineExecutor>()) {
  }

  IExecutorPtr Get() {
    return e_;
  }

 private:
  IExecutorPtr e_;
};

IExecutorPtr GetInlineExecutor() {
  static Instance single;
  return single.Get();
}

};  // namespace tiny::executors
