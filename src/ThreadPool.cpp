#include "ThreadPool.h"

ThreadPool::ThreadPool(unsigned int count) : run_(true), thread_count_(count) {
  for (int i = 0; i < count; ++i) {
    threads_.emplace_back([this]() {
      while (true) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(mutex_);
          // 线程池停止工作或者任务队列不为空则不阻塞
          cv_.wait(lock, [this]() { return !run_.load() || !tasks_.empty(); });
          if (!run_.load() && tasks_.empty()) {
            return;
          }
          task = std::move(tasks_.front());
          tasks_.pop();
        }
        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  run_.store(false);
  cv_.notify_all();
  for (auto& th : threads_) {
    if (th.joinable()) {
      th.join();
    }
  }
}
