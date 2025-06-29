#ifndef MULTITHREADING_HPP
#define MULTITHREADING_HPP

#include <vector>
#include <thread>
#include <functional>


/** Namespace for parallel, async operations */
namespace conc {

    /**
    * Parallel multithread realisation of for loop.No Concurrency.No Common resources.
    * Typical Loop: for (int start_index = 0; start_index < end_index; ++start_index) {}
    * If need to wrap member function void MemberFunction(int, int) of class A of object obj:
    * std::function<void(int, int)> func{ std::bind(&A::MemberFunction, &obj, std::placeholders::_1, std::placeholders::_2) };
    * multithreading::for_parallel(func, 0, max_iteration);
    * For loop func: for (; istart < imax; ++istart) {}
    */
    template<typename FuncT, typename... ArgsT>
    void for_parallel(std::function<FuncT> for_loop_func, int start_index, const int end_index, ArgsT... args) {
        unsigned int max_threads_count{ std::thread::hardware_concurrency() };
        std::vector<std::thread> running_threads{};

        const int iterations_count{ end_index - start_index };
        unsigned int loop_len{ iterations_count / max_threads_count };
        if (iterations_count <= max_threads_count) { // Not all threads will be used
            loop_len = 1;
            max_threads_count = iterations_count;
        }

        for (int thr = 0; thr < max_threads_count; ++thr) { // divide big loop into threads
            if (thr < (max_threads_count - 1)) { // for all threads except the last
                running_threads.emplace_back(for_loop_func, start_index + thr * loop_len, start_index + (thr + 1) * loop_len, args...);
            } else { // last thread
                running_threads.emplace_back(for_loop_func, start_index + thr * loop_len, end_index, args...);
            }
        }

        // Wait all threads execute
        for (int i = 0, imax = running_threads.size(); i < imax; ++i) {
            running_threads[i].join();
        }
        //running_threads.clear();
    }

    /**
    * Parallel multithread realisation of for loop.No Concurrency.No Common resources.
    * Typical Loop: for (int start_index = 0; start_index < end_index; ++start_index) {}
    */
    template<typename FuncT, typename... ArgsT>
    void FuncInvokeParallel(std::function<FuncT> func, ArgsT... args) {
        std::vector<std::thread> running_threads{};

        // Wait all threads execute
        for (int i = 0, imax = std::thread::hardware_concurrency(); i < imax; ++i) {
            running_threads.emplace_back(func, args...);
        }

        // Wait all threads execute
        for (int i = 0, imax = std::thread::hardware_concurrency(); i < imax; ++i) {
            running_threads[i].join();
        }
    }

} // !namespace conc

#endif // !MULTITHREADING_HPP