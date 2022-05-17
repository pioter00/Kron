#ifndef KRON_TASK_LIST_H
#define KRON_TASK_LIST_H
#include "Task.h"
#include <vector>
class TaskList {
private:
    std::vector<Task> taskList;
    static long counter;
    static long generateID();
public:
    void stop();

    void add(std::string &programName, std::string &programArgs,std::string &timeoutString, bool repetitions = false);
    bool remove(long id);

    std::string show();
};


#endif //KRON_TASK_LIST_H
