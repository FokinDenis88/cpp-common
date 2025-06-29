#include <iostream>
#include <chrono>
//#include <mutex>
//#include <exception>
//#include <stdexcept>

#include "concurrency-support-library/multithreading.hpp"


// Parralel For Loop Evaluation. There is no common resource. No Concurrency.

constexpr long long max_iteration{ 2000000000 };
//constexpr long long max_iteration{ 3 };

void DoSomething() {
    // Do something
    long long a = 3333333333333333;
    a = -1111111111111111;
    a = +2222222222222222;
}

class A {
public:
    void ForLoop(int i, int imax) {
        for (; i < imax; ++i) {
            DoSomething();
        }
    }
};

void ForLoop() {
    for (int i = 0, imax = max_iteration; i < imax; ++i) {
        DoSomething();
    }
}

void MultiThreadForLoopThread(int i, int imax) {
    for (; i < imax; ++i) {
        DoSomething();
    }
}

void MultiThreadForLoop() {
    unsigned int max_threads_count{ std::thread::hardware_concurrency() };
    std::vector<std::thread> running_threads{};

    const int maxi{ max_iteration }; // Loop max iteration
    unsigned int loop_len{ maxi / max_threads_count };
    if (maxi <= max_threads_count) { // Not all threads will be used
        loop_len = 1;
        max_threads_count = maxi;
    }

    for (int thr = 0; thr < max_threads_count; ++thr) { // divide big loop into threads
        if (thr < (max_threads_count-1)) {
            running_threads.emplace_back(&MultiThreadForLoopThread, thr * loop_len, (thr + 1) * loop_len);
            //MultiThreadForLoopThread(thr * loop_len, (thr + 1) * loop_len);
        } else {
            running_threads.emplace_back(&MultiThreadForLoopThread, thr * loop_len, maxi);
            //MultiThreadForLoopThread(thr * loop_len, maxi);
        }
    }

    // Wait all threads execute
    for (int i = 0, imax = running_threads.size(); i < imax; ++i) {
        running_threads[i].join();
    }
    running_threads.clear();
}

int Run() {
    auto start{ std::chrono::steady_clock::now() };
    //ForLoop();
    auto end{ std::chrono::steady_clock::now() };
    std::cout << "First Loop end\n";
    auto elapse_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();


    start = std::chrono::steady_clock::now();
    //MultiThreadForLoop();
    end = std::chrono::steady_clock::now();
    std::cout << "Second Loop end\n";
    auto elapse_time_thread = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();


    std::cout << "Miliseconds\n";
    std::cout << elapse_time << '\n' << elapse_time_thread << '\n';
    std::cout << "Threads are quicker in " << static_cast<long double>(elapse_time) / elapse_time_thread << " time.\n";

    start = std::chrono::steady_clock::now();
    //multithreading::for_parallel(std::function<void (int, int)>(MultiThreadForLoopThread), 0, max_iteration);
    end = std::chrono::steady_clock::now();
    std::cout << "Third Loop end\n";
    auto elapse_time_thread_template = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << elapse_time_thread_template << '\n';

    start = std::chrono::steady_clock::now();
    A obj{};
    std::function<void(int, int)> func{ std::bind(&A::ForLoop, &obj, std::placeholders::_1, std::placeholders::_2) };
    conc::for_parallel(func, 0, max_iteration);
    //std::function<void(A::*)(int, int)> func = A::ForLoop;
    /*std::function<void(const A&, int, int)> func{ &A::ForLoop };
    multithreading::for_parallel(func, 0, max_iteration);*/
    end = std::chrono::steady_clock::now();
    std::cout << "Third Loop end\n";
    auto elapse_time_thread_template2 = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << elapse_time_thread_template2 << '\n';
    return 0;
}

//if (maxj <= max_threads_count) { // Not all threads will be used
//    for (int j = 0; j < maxj; ++j) {
//        running_threads.emplace_back(&DoSomething);
//    }
//}
//else {
