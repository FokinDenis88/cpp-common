#ifndef THREAD_HPP
#define THREAD_HPP

//#include <string>	// CustomException
//#include <exception> // CustomException
//#include <stdexcept> // different exception types

namespace common {

	namespace thread {
		/*
	#include <thread>
	#include <vector>
	#include <queue>
	#include <functional>
	#include <future>
	#include <mutex>
	#include <condition_variable>
	#include <atomic>

				class ThreadPool {
				private:
					using Task = std::function<void()>;

					std::vector<std::thread> workers_;           // Рабочие потоки
					std::queue<Task> task_queue_;                 // Очередь задач
					std::mutex queue_mutex_;                      // Мьютекс для защиты очереди
					std::condition_variable condition_;          // Условная переменная для уведомления потоков
					std::atomic<bool> stop_{ false };              // Флаг остановки потока

				public:
					explicit ThreadPool(size_t thread_count)
						: workers_(thread_count) {
						for (size_t i = 0; i < thread_count; ++i) {
							workers_[i] = std::thread([this]() {
								while (!stop_) {
									Task task;
									{
										std::unique_lock<std::mutex> lock(queue_mutex_);
										condition_.wait(lock, [this]() { return !task_queue_.empty() || stop_; });

										if (stop_ && task_queue_.empty()) break;

										task = std::move(task_queue_.front());
										task_queue_.pop();
									}

									task();  // Выполняем задачу вне мьютекса
								}
								});
						}
					}

					~ThreadPool() {
						stop_ = true;
						condition_.notify_all();
						for (auto& worker : workers_)
							worker.join();
					}

					template<typename F, typename... Args>
					auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
						using ReturnType = typename std::invoke_result<F, Args...>::type;

						auto task = std::make_shared<std::packaged_task<ReturnType()>>(
							std::bind(std::forward<F>(f), std::forward<Args>(args)...));

						std::future<ReturnType> result = task->get_future();
						{
							std::lock_guard<std::mutex> lock(queue_mutex_);
							task_queue_.emplace([task]() { (*task)(); });
						}
						condition_.notify_one();
						return result;
					}

					void shutdown() {
						stop_ = true;
						condition_.notify_all();
						for (auto& worker : workers_)
							worker.join();
					}
				};

				// Простое использование
				int main() {
					ThreadPool pool(4); // Используем 4 потока

					// Добавляем задачу в пул
					auto future = pool.enqueue([]() {
						std::this_thread::sleep_for(std::chrono::seconds(2)); // Имитация длительной задачи
						return 42;
						});

					// Ждем завершения задачи
					int result = future.get();
					std::cout << "Result is: " << result << std::endl;

					return 0;
				}

				*/

		class TasksQueue {

		};

		class ThreadPool {
			// Best count of threads is: max threads of CPU + 1 (or 2)
		};
		/*
		* Container choices:
		* 1) vector
		* This is the most common and simplest option due to its ease of memory management and good performance
		* characteristics. A thread pool is usually implemented in such a way that a fixed number of threads are
		* allocated in advance and stored in a vector.
		*
		* It is important to note that inserting and deleting elements from the middle of a vector can cause all
		* subsequent elements to be moved, which slows down the operation. However, this problem can be solved
		* by pre-reserving memory (reserve()).
		*
		* 2) list, forward_list
		* If you expect threads to be frequently removed from arbitrary positions in the list (e.g. one thread has
		* finished its work and is removed from the pool), a doubly linked list (std::list) is a better choice, since
		* it provides constant time for removing an element across iterators.
		* Note: Make sure to use mutex protection to safely access the list from different threads.
		*
		* 3) deque
		* A queue allows you to efficiently add and remove items from both the front and back of a queue. It is especially
		* useful if you need to implement a priority task queue where items can be added and removed asynchronously by
		* different threads.
		*
		* 4) Intel TBB (Threaded Building Blocks) Containers for multithread work.
		*/

	} // !namespace thread

} // !namespace common


#endif // !THREAD_HPP
