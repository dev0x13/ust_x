#pragma once

#include <condition_variable>
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>
#include <queue>
#include <functional>

namespace UST {
  namespace Multithreading {
    class ThreadPool {
    private:
      bool done;
      std::condition_variable cond;
      size_t numThreads = 0;
      std::vector<std::thread> threads;
      std::mutex m;
      std::queue<std::function<void()>> tasks;

      volatile size_t blockSize;
    public:
      ThreadPool() {
        done = false;

       // logger << "Starting thread pool...\n";
        numThreads = std::thread::hardware_concurrency();

        if (numThreads == 0) {
          //logger << "Unable to get hardware concurrency support information. Using 1 thread\n";
          numThreads = 1;
        }

        for (size_t i = 0; i < numThreads; ++i) {
          threads.emplace_back([&] {
            for (;;) {
              std::function<void()> task;

              {
                std::unique_lock<std::mutex> lock(this->m);

                this->cond.wait(lock, [this] { return this->done || !this->tasks.empty(); });

                if (this->done && this->tasks.empty()) {
                  return;
                }

                task = std::move(this->tasks.front());
                this->tasks.pop();
              }

              task();
              {
                std::unique_lock<std::mutex> lock(this->m);

                blockSize--;
              }

            }
          });
        }

        //logger << numThreads << " concurrent threads are supported\n" << SEPARATOR;
      }

      template<class Function, class... Args>
      void runTask(Function&& f, Args&&... args) {
        auto task = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
        {
          std::unique_lock<std::mutex> lock(m);

          tasks.emplace(task);
        }
        cond.notify_one();
      }

      // Method for creating barrier by tasks counter e.g. for output
      void startTaskBlock(size_t size) {
        {
          std::unique_lock<std::mutex> lock(m);
         
          blockSize = size;
        }
      }

      void terminate() {
        {
          std::unique_lock<std::mutex> lock(m);

          done = true;
        }
        cond.notify_all();

        for (std::thread &t : threads) {
          t.join();
        }
      }

      void wait() {
        while (blockSize > 0) {}
        {
          std::unique_lock<std::mutex> lock(m);

          blockSize = 0;
        }
      }

      size_t getNumThreads() {
        return numThreads;
      }

      ~ThreadPool() {
        terminate();
      }
    };
  }
}
