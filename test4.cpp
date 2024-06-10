#include <iostream>
#include <thread>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <mutex>
#include <vector>
#include <sys/wait.h>
#include <spawn.h>
#include <fcntl.h>
#include <sched.h>
#include <pthread.h>

static std::chrono::steady_clock::time_point last;
std::mutex mtx;

// 1GB veri alanı
std::vector<int> largeData(250 * 1024 * 1024 *2, 1); // Her int 4 byte, toplam 1GB

void setThreadPriority(int policy, int priority) {
    struct sched_param schedparam;
    schedparam.sched_priority = priority;

    pthread_t pthreadID = pthread_self();
    if (pthread_setschedparam(pthreadID, policy, &schedparam) != 0) {
        perror("pthread_setschedparam");
    }
}

void threadFunction(int id) {
    int policy = SCHED_FIFO; // FIFO zamanlama politikası
    int priority = sched_get_priority_max(SCHED_FIFO); // En yüksek öncelik
    // priority = sched_get_priority_min(SCHED_FIFO); // En yüksek öncelik

    // Thread önceliğini ayarla
    setThreadPriority(policy, priority);

    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = now - last;
        double diff = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count();
        if (diff > 1000) { // 10 milisaniyeden büyükse yazdır
            std::cout << "Thread " << id << " elapsed time: " << diff << " microseconds" << std::endl;
        }
        std::cout << diff << std::endl;
        last = now;
        lock.unlock();
        // for (volatile int i = 0; i < 10000; ++i); // CPU yoğun bir iş yap
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 10 milisaniye uyuma
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
        char *args[] = {"sleep", "2", NULL}; // Örnek olarak 'echo' komutunu çalıştır
        execvp("sleep", args);
        // exec başarısız olursa
        perror("exec failed");
        _exit(1);
    } else {
        // Ebeveyn süreç
        wait(NULL); // Çocuk sürecin bitmesini bekle
    }
}

void executeWithForkAndExec() {
    pid_t pid = fork();
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

void executeWithPosixSpawn() {
    posix_spawn_file_actions_t file_actions;
    posix_spawnattr_t attr;
    pid_t pid;
    char *argv[] = {"echo", "Hello", NULL}; // Örnek olarak 'echo' komutunu çalıştır
    sigset_t sigmask, sigdefault;
    struct sched_param schedparam;

    // posix_spawn_file_actions_t ve posix_spawnattr_t yapısını başlat
    if (posix_spawn_file_actions_init(&file_actions) != 0) {
        perror("posix_spawn_file_actions_init");
        return;
    }

    if (posix_spawnattr_init(&attr) != 0) {
        perror("posix_spawnattr_init");
        posix_spawn_file_actions_destroy(&file_actions);
        return;
    }

    // Sinyal maskesini ayarla
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGINT); // SIGINT (Ctrl+C) sinyalini maskeye ekle
    if (posix_spawnattr_setsigmask(&attr, &sigmask) != 0) {
        perror("posix_spawnattr_setsigmask");
        posix_spawn_file_actions_destroy(&file_actions);
        posix_spawnattr_destroy(&attr);
        return;
    }

    // Varsayılan sinyal işleyicilerini ayarla
    sigemptyset(&sigdefault);
    sigaddset(&sigdefault, SIGTERM); // SIGTERM sinyalini varsayılan işleyiciye ayarla
    if (posix_spawnattr_setsigdefault(&attr, &sigdefault) != 0) {
        perror("posix_spawnattr_setsigdefault");
        posix_spawn_file_actions_destroy(&file_actions);
        posix_spawnattr_destroy(&attr);
        return;
    }

    // Düşük öncelik ayarla
    schedparam.sched_priority = sched_get_priority_min(SCHED_OTHER); // En düşük öncelik
    if (posix_spawnattr_setschedpolicy(&attr, SCHED_OTHER) != 0) {
        perror("posix_spawnattr_setschedpolicy");
        posix_spawn_file_actions_destroy(&file_actions);
        posix_spawnattr_destroy(&attr);
        return;
    }

    if (posix_spawnattr_setschedparam(&attr, &schedparam) != 0) {
        perror("posix_spawnattr_setschedparam");
        posix_spawn_file_actions_destroy(&file_actions);
        posix_spawnattr_destroy(&attr);
        return;
    }

    // Yeni bir süreç oluştur
    if (posix_spawn(&pid, "/bin/echo", &file_actions, &attr, argv, environ) != 0) {
        perror("posix_spawn");
    } else {
        // Ebeveyn süreç: çocuk sürecin bitmesini bekle
        waitpid(pid, NULL, 0);
    }

    // posix_spawn_file_actions_t ve posix_spawnattr_t yapısını temizle
    posix_spawn_file_actions_destroy(&file_actions);
    posix_spawnattr_destroy(&attr);
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

    // Daha fazla thread oluştur
    std::vector<std::thread> threads;
    for (int i = 0; i < 1; ++i) {
        threads.emplace_back(threadFunction, i);
    }

    for (int i = 0; i < 1000; ++i) {
        auto start = std::chrono::steady_clock::now();
        executeWithPosixSpawn(); // posix_spawn fonksiyonunu kullan
        // executeWithForkAndExec();
        // executeWithVforkAndExec();
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        double diff = std::chrono::duration_cast<std::chrono::microseconds>(elapsed_seconds).count();
        std::cout << "posix_spawn time: " << diff << " microseconds" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 10 milisaniye uyuma
    }

    for (auto &thread : threads) {
        thread.join();
    }

    return 0;
}
