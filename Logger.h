#ifndef LOGGER_LOGGER_H
#define LOGGER_LOGGER_H

#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <csignal>
#include <cstring>
#include <atomic>
#include <semaphore.h>

class LoggerExistsException : public std::exception {
    const char* what() const noexcept override {
        return "Object Logger already exists!\n";
    }
};

typedef enum {
    INFO,
    ERROR,
    DEBUG
} log_type_t;

typedef enum {
    NONE,
    MINIMUM,
    STANDARD,
    ALL
} log_details_t;

typedef enum {
    MINIMUM_PRIORITY,
    BASIC_PRIORITY,
    MAXIMUM_PRIORITY
} log_priority_t;

class Logger {
private:
    std::ofstream file_to_log;
    void *memory_to_dump{nullptr};
    size_t size_of_memory_to_dump{0};
    std::string file_to_dump_location;
    void dump_to_file();

    static int instances_count;
    static std::atomic<int> dump_count;
    static std::atomic<int> details;
    static sem_t semaphore_dump;
    static std::string current_time_to_string();
    static pthread_mutex_t save_logs_mutex;

    friend void* signal_dump_to_file(void* arg);
    friend void handler_set_details(int signo, siginfo_t* info, void* other);
    friend void handler_dump(int signo);

    template<typename T>
    void save_variables_to_file(T t) {
        file_to_log << t << " ";
    }

    template<typename T, typename... Args>
    void save_variables_to_file(T t, Args... args) {
        file_to_log << t << " ";
        save_variables_to_file(args...);
    }

public:

    explicit Logger(const std::string &filename);

    ~Logger();

    void config_dump(void *memory_ptr, size_t size, const std::string &location = "");

    template<typename... Args>
    void log(log_type_t type, log_priority_t priority, Args... args) {
        if (details == NONE ||
            (priority == MINIMUM_PRIORITY && (details == MINIMUM || details == STANDARD)) ||
            (priority == BASIC_PRIORITY && details == MINIMUM))
            return;
        std::cout << "priority: " << priority << " this->details: " << details << std::endl;
        pthread_mutex_lock(&save_logs_mutex);
        switch (details) {
            case MINIMUM:
                break;
            case STANDARD:
                file_to_log << current_time_to_string() << std::endl;
                break;
            case ALL:
                file_to_log << __FILE__ << " " << __LINE__ << std::endl;
                file_to_log << current_time_to_string() << std::endl;
                break;
            default:
                return;
        }
        switch (type) {
            case INFO:
                save_variables_to_file("INFO: ", args...);
                file_to_log << std::endl;
                break;
            case ERROR:
                save_variables_to_file("ERROR: ", args...);
                file_to_log << std::endl;
                break;
            case DEBUG:
                save_variables_to_file("DEBUG: ", args...);
                file_to_log << std::endl;
                break;
        }
        pthread_mutex_unlock(&save_logs_mutex);
    }
};

void* signal_dump_to_file(void* arg);
void handler_set_details(int signo, siginfo_t* info, void* other);
void handler_dump(int signo);
#endif
