#include <iostream>
#include <thread>
#include <chrono>
#include <pthread.h>
#include <vector>

void threadFunction(long priority, long thread_id) {
    // İş parçacığı önceliği ayarlama
    struct sched_param param;
    param.sched_priority = priority;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    for (long i = 0; i < 10000000000; ++i) {
        // auto start = std::chrono::steady_clock::now();
        // std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 1 milisaniye uyuma
        // auto end = std::chrono::steady_clock::now();
        // std::chrono::duration<double> elapsed_seconds = end - start;
        // double diff = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count();
        // std::cout << "Thread " << thread_id << " with priority " << priority << " iteration " << i 
        //           << " elapsed time: " << diff << " microseconds" << std::endl;
    }
    std::cout << thread_id << "fin\n"; 
}

void threadFunction1(long priority, long thread_id) {
    // İş parçacığı önceliği ayarlama
    struct sched_param param;
    param.sched_priority = priority;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    for (long i = 0; i < 10000000000; ++i) {
        // auto start = std::chrono::steady_clock::now();
        // std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 1 milisaniye uyuma
        // auto end = std::chrono::steady_clock::now();
        // std::chrono::duration<double> elapsed_seconds = end - start;
        // double diff = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count();
        // std::cout << "Thread " << thread_id << " with priority " << priority << " iteration " << i 
        //           << " elapsed time: " << diff << " microseconds" << std::endl;
    }
    std::cout << thread_id << "fin\n"; 
}

void threadFunction2(long priority, long thread_id) {
    // İş parçacığı önceliği ayarlama
    struct sched_param param;
    param.sched_priority = priority;
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    for (long i = 0; i < 10000000000; ++i) {
        // auto start = std::chrono::steady_clock::now();
        // // std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 1 milisaniye uyuma
        // auto end = std::chrono::steady_clock::now();
        // std::chrono::duration<double> elapsed_seconds = end - start;
        // double diff = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count();
        // std::cout << "Thread " << thread_id << " with priority " << priority << " iteration " << i 
        //           << " elapsed time: " << diff << " microseconds" << std::endl;
    }
    std::cout << thread_id << "fin\n"; 
}

int main() {
    // Üç iş parçacığı oluşturma
    std::thread thread1(threadFunction1, 50, 1);  // Orta öncelik
    std::thread thread0(threadFunction, 1, 0);   // Düşük öncelik
    std::thread thread2(threadFunction2, 99, 2);  // Yüksek öncelik

    // İş parçacıklarının bitmesini bekle
    thread0.join();
    thread1.join();
    thread2.join();

    return 0;
}
