#include "TaskList.h"

long TaskList::counter = 0;

long TaskList::generateID() {
    return TaskList::counter;
}

void TaskList::add(std::string &programName, std::string &programArgs,std::string &timeoutString, bool repetitions) {
    try {
        taskList.emplace_back(generateID(), programName, programArgs, Task::getTimeoutFromString(timeoutString), repetitions);
    } catch (std::exception& e) {
        throw e;
    }
    TaskList::counter++;
}

bool TaskList::remove(long id) {
    auto temp = taskList;
    long index = 0;
    for(const auto& element : temp) {
        if(element.getId() == id){
            std::cout << id << std::endl;
            taskList[index].cancel();
            taskList.erase(taskList.begin() + index);
            return true;
        }
        index++;
    }
    return false;
}

std::string TaskList::show() {
    std::string buffer;
    for(auto element : taskList) {
        buffer += element.toString();
    }
    std::cout << "Buffer length: "<< buffer.length() << std::endl;
    return !buffer.empty() ? buffer : "Empty list!\n";
}


void TaskList::stop() {
    auto temp = taskList;
    for(const auto& element : temp)
        remove(element.getId());
}
