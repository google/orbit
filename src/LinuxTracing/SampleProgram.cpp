#include <absl/strings/str_format.h>
#include <string.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

static std::atomic<bool> KEEP_GOING;
static std::atomic<int> num;

std::mutex mtx;

constexpr int kNumOfThreads = 4;

void DoWork(int thread_num) {
    std::string thread_name = absl::StrFormat(
        "Thread %d",
        thread_num
    );
    pthread_setname_np(pthread_self(), thread_name.c_str());
    while (true) {
        if (!KEEP_GOING.load()) {
            break;
        }
        int temp = num;
        std::lock_guard<std::mutex> lockit(mtx);
        while (temp == num);
    }
}

int main() {
    std::thread my_threads[kNumOfThreads];
    KEEP_GOING.store(true);
    for(int i = 0; i < kNumOfThreads; i++) {
        my_threads[i] = std::thread(&DoWork, i);
    }
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        num++;
    }
    KEEP_GOING.store(false);
    for(auto & my_thread : my_threads) {
        my_thread.join();
    }
}