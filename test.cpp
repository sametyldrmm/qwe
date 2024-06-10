#include <iostream>
#include <thread>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <mutex>
#include <pthread.h>
#include <vector>
#include <sys/wait.h>

static std::chrono::steady_clock::time_point last;
std::mutex mtx;

// 1GB veri alanı
// std::vector<int> largeData(250 * 1024 * 1024, 1); // Her int 4 byte, toplam 1GB

void threadFunction() {
        std::cout << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" << std::endl;

    // İş parçacığı önceliği ayarlama
    struct sched_param param;
    param.sched_priority = 1; // Yüksek öncelik seviyesi
    pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = now - last;
        double diff = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count();
        if (diff >= 0) { // 10 milisaniyeden büyükse yazdır
            std::cout << "Elapsed time: " << diff << " microseconds" << std::endl;
        }
        last = now;
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 10 milisaniye uyuma
    }
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

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
}

void executeWithForkAndExec() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
    } else if (pid == 0) {
        // Çocuk süreç
        char *args[] = {"echo", "Hello", "1>/dev/null", NULL}; // Örnek olarak 'echo' komutunu çalıştır
        execvp("echo", args);
        // exec başarısız olursa
        perror("exec failed");
        exit(1);
    } else {
        // Ebeveyn süreç
        wait(NULL); // Çocuk sürecin bitmesini bekle
    }
}

double measureSystemCall() {
    auto start = std::chrono::steady_clock::now();
    int status = system("echo Hello 1>/dev/null");
    auto end = std::chrono::steady_clock::now();
    if (status == -1) {
        perror("system call failed");
    }
    std::chrono::duration<double> elapsed_seconds = end - start;
    return std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count();
}

double measureForkAndExec() {
    auto start = std::chrono::steady_clock::now();
    executeWithForkAndExec();
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    return std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count();
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

    double totalSystemTime = 0;
    double totalForkExecTime = 0;
    int iterations = 100;

    for (int i = 0; i < iterations; ++i) {
        totalSystemTime += measureSystemCall();
        totalForkExecTime += measureForkAndExec();

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms); // 10 milisaniye uyuma
    }

    double avgSystemTime = totalSystemTime / iterations;
    double avgForkExecTime = totalForkExecTime / iterations;

    std::cout << "Average system() call time: " << avgSystemTime << " microseconds" << std::endl;
    std::cout << "Average fork() + exec() call time: " << avgForkExecTime << " microseconds" << std::endl;
    exit(2);
    thread.join();

    return 0;
}

// real    0m6,610s         real    0m6,612s    verage system() call time: 1073.41 microseconds
// user    0m0,757s         user    0m0,803s    Average fork() + exec() call time: 45405.1 microseconds
// sys     0m4,847s         sys     0m4,794s    


// real    0m6,044s
// user    0m0,817s
// sys     0m4,213s


// priority 1
// real    0m6,949s
// user    0m0,830s // Average system() call time: 1435.48 microseconds
// sys     0m5,107s // Average fork() + exec() call time: 47837.5 microseconds

// real    0m6,324s
// user    0m0,807s Average system() call time: 1251.3 microseconds
// sys     0m4,507s Average fork() + exec() call time: 42317.8 microseconds