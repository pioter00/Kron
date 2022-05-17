#ifndef KRON_TASK_H
#define KRON_TASK_H
#include <iostream>
struct data_t {
    char *name;
    char **args;
};
class Task {

private:
    long _id;
    std::string _program;
    long _timeout;
    bool _repetitions;
    timer_t timer_id{};
    struct data_t data{};

public:
    static long getTimeoutFromString(const std::string& time);
    static std::string getStringFromTimeout(long timeout);
    Task(long id, std::string& programName, std::string& programArgs, long timeout, bool repetitions = false);
    long getId() const;
    void cancel();
    std::string toString();
    friend std::ostream& operator<< (std::ostream& o, const Task& task) {
        o << "Task #" << task._id << " program: " << task._program <<
          " at: " << Task::getStringFromTimeout(task._timeout) <<
          " " << task._repetitions << std::endl;
        return o;
    }
};


#endif //KRON_TASK_H
