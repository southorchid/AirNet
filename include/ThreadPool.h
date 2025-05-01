#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
 public:
  ThreadPool(unsigned int count = std::thread::hardware_concurrency());
  ~ThreadPool();

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  template <class F, class... Args>
  auto enqueue(F&& f, Args&&... args)
      -> std::future<typename std::invoke_result<F, Args...>::type>;

 private:
  std::atomic<bool> run_;
  int thread_count_;
  std::vector<std::thread> threads_;
  std::queue<std::function<void()>> tasks_;
  std::mutex mutex_;
  std::condition_variable cv_;
};

template <class F, class... Args>
inline auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result<F, Args...>::type> {
  // 定义任务函数返回的类型
  using return_type = typename std::invoke_result<F, Args...>::type;

  // 打包任务函数
  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> result = task->get_future();
  {
    std::unique_lock<std::mutex> lock(mutex_);
    if (run_.load()) {
      tasks_.emplace([task]() { (*task)(); });
    }
  }
  cv_.notify_one();
  return result;
}
