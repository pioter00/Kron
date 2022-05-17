#include "Task.h"
#include <ctime>
#include <csignal>
#include <cstring>
#include <algorithm>
#include <spawn.h>

void timer_thread(union sigval timer_data) {
    auto programData = reinterpret_cast<struct data_t *>(timer_data.sival_ptr);
    printf("%s\n", programData->name);
    for (int i = 0; programData->args[i]; i++) {
        printf("a: %s\n", programData->args[i]);
    }
    pid_t processID;
    int status = posix_spawn(&processID,programData->name, nullptr, nullptr,programData->args, nullptr);
    if (status)
        std::cout << "Error in executing program" << std::endl;
}

long Task::getId() const {
    return _id;
}

long Task::getTimeoutFromString(const std::string& time) {
    struct timespec current_time{};
    clock_gettime(CLOCK_REALTIME, &current_time);
    try {
        long temp = std::stol(time);
        return current_time.tv_sec + temp;
    } catch (...) {
        tm *ltm = localtime(&current_time.tv_sec);
        if ((time.c_str(), "%H:%M:%S", ltm))
            return mktime(ltm);
        return -1;
    }
}

void Task::cancel() {
    delete[] data.name;
    for (int i = 0; data.args[i]; i++)
        delete[] data.args[i];
    delete[] data.args;
    struct itimerspec value{0, 0, 0, 0};
    timer_settime(timer_id, TIMER_ABSTIME, &value, nullptr);
}

std::string Task::getStringFromTimeout(long timeout) {
    char buffer[255];
    tm *ltm = localtime(&timeout);
    strftime(buffer, sizeof(buffer), "%d %b %Y %H:%M:%S", ltm);
    std::string timeoutString(buffer);
    return timeoutString;
}

Task::Task(long id, std::string &programName, std::string &programArgs, long timeout, bool repetitions) {
    _id = id;
    _program = programName + " " + programArgs;
    _timeout = timeout;
    _repetitions = repetitions;
    data.name = new char[programName.length()];
    long argc = std::count(programArgs.begin(), programArgs.end(), ' ') + 3;
    std::cout << "ARGC " << argc << std::endl;
    data.args = new char*[argc];
    size_t pos = 0;
    std::string token;
    std::string delimiter = " ";
    size_t index = 1;
    strcpy(data.name, programName.c_str());
    data.args[0] = new char [programName.length()];
    strcpy(data.args[0], programName.c_str());
    if (programArgs.empty())
        data.args[1] = nullptr;
    else if (argc == 3) {
        data.args[1] = new char [programArgs.length()];
        strcpy(data.args[1], programArgs.c_str());
        data.args[2] = nullptr;
    } else {
        programArgs += delimiter;
        while ((pos = programArgs.find(delimiter)) != std::string::npos) {
            token = programArgs.substr(0, pos);
            std::cout << token << std::endl;
            data.args[index] = new char [token.length()];
            strcpy(data.args[index++], token.c_str());
            programArgs.erase(0, pos + delimiter.length());
        }
        data.args[index] = nullptr;
    }
    struct sigevent timer_event2{};
    timer_event2.sigev_notify = SIGEV_THREAD;
    timer_event2.sigev_notify_function = timer_thread;
    timer_event2.sigev_value.sival_ptr = (void *)&data;
    timer_event2.sigev_notify_attributes = nullptr;
    if (timer_create(CLOCK_REALTIME, &timer_event2, &timer_id) < 0) {
        throw std::runtime_error("Error creating timer!");
    }
    struct itimerspec value{_repetitions ? _timeout : 0, 0, _timeout, 0};

	if (timer_settime(timer_id, TIMER_ABSTIME, &value, nullptr) < 0) {
        throw std::runtime_error("Error setting timer!");
    }
}

std::string Task::toString() {
    return "Task #" + std::to_string(_id) + " program: " + _program +
                     " at: " + Task::getStringFromTimeout(_timeout) +
                     " " + std::to_string(_repetitions) + "\n";
}
