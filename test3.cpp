#include <iostream>
#include <thread>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <mutex>
#include <vector>
#include <sys/wait.h>

static std::chrono::steady_clock::time_point last;
std::mutex mtx;

// 1GB veri alanı
std::vector<int> largeData(250 * 1024 * 1024, 1); // Her int 4 byte, toplam 1GB

void threadFunction() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = now - last;
        double diff = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count();
        if (diff > 10000) { // 10 milisaniyeden büyükse yazdır
            std::cout << "Elapsed time: " << diff << " microseconds" << std::endl;
        }
        last = now;
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 10 milisaniye uyuma
    }
}

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
}

void executeWithVforkAndExec() {
    pid_t pid = vfork();
    if (pid < 0) {
        perror("vfork failed");
    } else if (pid == 0) {
        // Çocuk süreç
        char *args[] = {"echo", "Hello", NULL}; // Örnek olarak 'echo' komutunu çalıştır
        execvp("echo", args);
        // exec başarısız olursa
        perror("exec failed");
        _exit(1);
    } else {
        // Ebeveyn süreç
        wait(NULL); // Çocuk sürecin bitmesini bekle
    }
}

int main() {
    // Sinyal işleyicileri
    signal(SIGABRT, signalHandler);
    signal(SIGBUS, signalHandler);
    signal(SIGSEGV, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);

    last = std::chrono::steady_clock::now(); // Başlangıç zamanını ayarla

    std::thread thread(threadFunction);

    for (int i = 0; i < 1000; ++i) {
        auto start = std::chrono::steady_clock::now();
        executeWithVforkAndExec();
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        double diff = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count();
        std::cout << "vfork + exec time: " << diff << " microseconds" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 10 milisaniye uyuma
    }

    thread.join();

    return 0;
}
