#include "Logger.h"
#include <unistd.h>
int Logger::instances_count = 0;
std::atomic<int> Logger::details;
std::atomic<int> Logger::dump_count;
sem_t Logger::semaphore_dump;
pthread_mutex_t Logger::save_logs_mutex;

std::string Logger::current_time_to_string() {
    char buffer[80];
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
    return buffer;
}

Logger::Logger(const std::string &filename) {
    if (++instances_count > 1) throw LoggerExistsException();
    try {
        this->file_to_log.open(filename);
    } catch (std::exception &e) {
        std::cout << "Invalid open file_to_log" << std::endl;
        throw e;
    }
    std::atomic_store(&Logger::details, 2);
    std::atomic_store(&Logger::dump_count, 0);
    pthread_mutex_init(&Logger::save_logs_mutex, nullptr);

    pthread_t tid1;

    struct sigaction act;

    sigset_t set;
    sigfillset(&set);

    act.sa_sigaction = handler_set_details;
    act.sa_mask = set;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGRTMIN, &act, nullptr);

    act.sa_handler = handler_dump;
    act.sa_mask = set;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGRTMIN+1, &act, nullptr);

    sigdelset(&set, SIGRTMIN);
    sigdelset(&set, SIGRTMIN+1);

    pthread_sigmask(SIG_BLOCK, &set, nullptr);

    pthread_create(&tid1, nullptr, signal_dump_to_file, this);
}

Logger::~Logger() {
    instances_count--;
    pthread_mutex_destroy(&Logger::save_logs_mutex);
    if (file_to_log.is_open())
        file_to_log.close();
}

void Logger::config_dump(void *memory_ptr, size_t size, const std::string &location) {
    this->memory_to_dump = memory_ptr;
    this->size_of_memory_to_dump = size;
    this->file_to_dump_location = location;
}

void Logger::dump_to_file() {
    const auto *p = reinterpret_cast<const unsigned char *>(memory_to_dump);
    char temp[31] = {'\0'};
    int counter = std::atomic_fetch_add(&Logger::dump_count, 1);
    std::ofstream file;
    std::string filename = file_to_dump_location + current_time_to_string() + " (" + std::to_string(counter) + ").log";
    try {
        file.open(filename);
    } catch (std::exception &e) {
        std::cout << "Invalid open " << filename << std::endl;
        throw e;
    }

    for (size_t i = 0; i < size_of_memory_to_dump; i++) {
        sprintf(temp, "%02X ", *p++);
        file << temp;
        if ((i + 1) % 10 == 0) file << std::endl;
    }
    file << std::endl;
    file.close();
}
void* signal_dump_to_file(void* arg) {
    auto *logger = static_cast<Logger*>(arg);
    while(true) {
        sem_wait(&Logger::semaphore_dump);
        std::cout << "signal to dump received" << std::endl;
        try {
            logger->dump_to_file();
        } catch (std::exception &e) {
            break;
        }
    }
    return nullptr;
}
void handler_set_details(int signo, siginfo_t* info, void* other){
    std::atomic_store(&Logger::details, info->_sifields._rt.si_sigval.sival_int);
}
void handler_dump(int signo){
    sem_post(&Logger::semaphore_dump);
}