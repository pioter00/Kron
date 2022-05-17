#ifndef KRON_CONSOLE_API_H
#define KRON_CONSOLE_API_H
#include <iostream>
#include <sys/wait.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <ctime>
#include "TaskList.h"
typedef enum {
    ADD,
    CANCEL,
    SHOW,
    HELP,
    QUIT,
    UNDEFINED
} userAction;

typedef struct {
    char queueName[20];
    char programName[30];
    char programArgs[100];
    char timeoutString[20];
    bool repetitions;
    userAction action;
    long cancelTaskID;
} ProgramDataClient;

typedef struct {
    char response[1000];
    int status;
} ServerResponse;

class ConsoleAPI {
private:
    TaskList taskList;
public:
    static ProgramDataClient getAction(int argc, char **argv);
    static void printHelp();
    std::string parseCommand(ProgramDataClient& programDataClient);
    ~ConsoleAPI();
};


#endif //KRON_CONSOLE_API_H
