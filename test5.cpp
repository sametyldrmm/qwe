#include <iostream>
#include <fstream>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include <chrono>
#include <vector>

// Zaman ölçümü için type alias
using clock_type = std::chrono::high_resolution_clock;
using time_point = std::chrono::time_point<clock_type>;

std::vector<int> largeData(250 * 1024 * 1024, 1); // Her int 4 byte, toplam 1GB

void executeWithVforkAndExec() {
    auto start_time = clock_type::now();
    while (true) {

        pid_t pid = vfork();

        if (pid < 0) {
            std::cerr << "Fork başarısız oldu" << std::endl;
            continue;
        }

        if (pid == 0) { // Çocuk işlem
            execl("/bin/ls", "ls", nullptr);
            std::cerr << "execl başarısız oldu: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        } else { // Ana işlem
            int status;
            waitpid(pid, &status, 0);
        }

        auto end_time = clock_type::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        std::cout << "forkAndLs çalışma süresi: " << duration << " ms" << std::endl;
        start_time = end_time;
        // std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void readWriteFile() {
    const char* filename = "example.txt";
    auto start_time = clock_type::now();
    while (true) {
        
        // Dosyayı oku
        std::ifstream infile(filename);
        std::string content((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
        infile.close();

        // std::cout << "Dosya içeriği: " << content << std::endl;

        // Dosyayı sıfırla
        std::ofstream outfile(filename, std::ofstream::trunc);
        outfile.close();

        // Yeni içerik yaz
        std::ofstream outfile_write(filename);
        outfile_write << "Yeni içerik\n";
        outfile_write.close();

        auto end_time = clock_type::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        // std::cout << "readWriteFile çalışma süresi: " << duration << " ms" << std::endl;
        start_time = end_time;
        // std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void forkAndLs() {
    auto start_time = clock_type::now();
    while (true) {

        pid_t pid = fork();

        if (pid < 0) {
            std::cerr << "Fork başarısız oldu" << std::endl;
            continue;
        }

        if (pid == 0) { // Çocuk işlem
            execl("/bin/ls", "ls", nullptr);
            std::cerr << "execl başarısız oldu: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        } else { // Ana işlem
            int status;
            waitpid(pid, &status, 0);
        }

        auto end_time = clock_type::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        std::cout << "forkAndLs çalışma süresi: " << duration << " ms" << std::endl;
        start_time = end_time;
        // std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    std::thread thread1(readWriteFile);
    std::thread thread2(executeWithVforkAndExec);

    thread1.join();
    thread2.join();

    return 0;
}
