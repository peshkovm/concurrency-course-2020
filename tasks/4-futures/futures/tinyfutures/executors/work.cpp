#include <tinyfutures/executors/work.hpp>

namespace tiny::executors {

class KeepWorkingExecutor : public IExecutor {
 public:
  KeepWorkingExecutor(IExecutorPtr e) : e_(std::move(e)) {
    e_->WorkCreated();
  }

  void Execute(Task&& task) override {
    e_->Execute(std::move(task));
  }

  void WorkCreated() override {
    e_->WorkCreated();
  }

  void WorkCompleted() override {
    e_->WorkCompleted();
  }

  ~KeepWorkingExecutor() {
    e_->WorkCompleted();
  }

 private:
  IExecutorPtr e_;
};

IExecutorPtr KeepWorking(IExecutorPtr e) {
  return std::make_shared<KeepWorkingExecutor>(std::move(e));
}

}  // namespace tiny::executors
