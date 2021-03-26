#pragma once
#include "threadpool.h"
#include "core/platform/EigenNonBlockingThreadPool.h"
#include <thread>
#include <atomic>

#define MAX_NUM_TASK 8

namespace onnxruntime {

namespace concurrency {

class ThreadPoolLite final : public ThreadPool {
 public:
  ThreadPoolLite(Env*,
                 const ThreadOptions&,
                 const NAME_CHAR_TYPE*,
                 int num_threads,
                 bool);
  ~ThreadPoolLite();

 private:
  using Fn = std::function<void(std::ptrdiff_t, std::ptrdiff_t)>;
  using SimpleFn = std::function<void(std::ptrdiff_t)>;
  using SchdFn = std::function<void()>;

  enum Status {
    Empty = 0,
    Loading,
    Ready
  };

  struct Task {
    std::atomic<Status> status_ = Empty;
    std::atomic_llong progress_ = 0;
    std::atomic_int done_ = 0;
    const SimpleFn* fn_ = nullptr;
    char padding_[64] = "\0";
  };

  int num_tasks = 0;
  Task tasks_[MAX_NUM_TASK];
  int NumThreads() const override { return num_sub_threads_; }
  void ParallelFor(std::ptrdiff_t, double, const Fn&) override;
  void ParallelFor(std::ptrdiff_t, const TensorOpCost&, const Fn&) override;
  void SimpleParallelFor(std::ptrdiff_t, const SimpleFn&) override;
  void Schedule(SchdFn) override;
  void StartProfiling() override;
  void MainLoop(int);
  std::string StopProfiling() override;
  int num_sub_threads_;
  std::vector<std::thread> sub_threads_;
  bool exit_ = false;
  ThreadPoolProfiler profiler_;
};

}  // namespace concurrency
}  // namespace onnxruntime