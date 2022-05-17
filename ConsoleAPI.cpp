#include "ConsoleAPI.h"
#include "cstring"
#include <unistd.h>
void ConsoleAPI::printHelp() {
    std::cout <<
              "Usage:\n"
              "\t-h prints this help\n"
              "\t-p 'program' set program\n"
              "\t-a 'arg1 arg2' set program args\n"
              "\t-d <delay> set delay\n"
              "\tDelay formats:\n"
              "\t\t '10' - program runs after 10 seconds\n"
              "\t\t '12:13:01' - program runs at 12:13:01\n"
              "\t-r set number of repetitions (you must combine this flag with -d 'seconds')\n"
              "\t-c <cancelTaskID> cancel program by cancelTaskID\n"
              "\t-s show all active programs\n"
              "\t-q quit app\n"
              "Example usages:\n"
              "\tkron -p 'ls' -a '-la' -d 10"
              << std::endl;
}
ProgramDataClient ConsoleAPI::getAction(int argc, char **argv) {
    std::string programName, programArgs, timeoutString;
    bool repetitions = false, id = -1;
    userAction action;
    switch (argc) {
        case 1: {
            std::string flag(argv[0]);
            if (flag == "-h")
                action = HELP;
            else if (flag == "-s")
                action = SHOW;
            else if (flag == "-q")
                action = QUIT;
            else action = UNDEFINED;
            break;
        }
        case 2: {
            std::string flag(argv[0]);
            if (flag != "-c"){
                action = UNDEFINED;
                break;
            }
            std::string value(argv[1]);
            try {
                id = std::stoi(value);
            } catch (...) {
                action = UNDEFINED;
                break;
            }
            action = CANCEL;
            break;
        }
        case 4:
        case 6:
        case 8: {
            for (int i = 0; i < argc; i+=2) {
                std::string flag(argv[i]);
                std::string value(argv[i + 1]);
                if (flag == "-p")
                    programName = value;
                else if (flag == "-a")
                    programArgs = value;
                else if (flag == "-d")
                    timeoutString = value;
                else if (flag == "-r") {
                    if (value == "yes")
                        repetitions = true;
                    else if (value == "no")
                        repetitions = false;
                    else action = UNDEFINED;
                } else action = UNDEFINED;
            }
            if (action == UNDEFINED)
                break;
            if (programName.empty() || timeoutString.empty())
                action = UNDEFINED;
            else action = ADD;
            break;
        }
        default:
            action = UNDEFINED;
    }
    ProgramDataClient programDataClient = {};
    std::string queueName = "/clientPID#" + std::to_string(getpid());
    memcpy(programDataClient.queueName, queueName.c_str(), queueName.length());
    memcpy(programDataClient.programName, programName.c_str(), programName.length());
    memcpy(programDataClient.programArgs, programArgs.c_str(), programArgs.length());
    memcpy(programDataClient.timeoutString, timeoutString.c_str(), timeoutString.length());
    programDataClient.repetitions = repetitions;
    programDataClient.action = action;
    programDataClient.cancelTaskID = id;

    return programDataClient;
}

ConsoleAPI::~ConsoleAPI() {
    try {
        taskList.stop();
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}

std::string ConsoleAPI::parseCommand(ProgramDataClient &programDataClient) {
    switch (programDataClient.action) {
        case ADD: {
            std::cout << "pn " << programDataClient.programName << " pa " << programDataClient.programArgs <<
                      " ts "<< programDataClient.timeoutString << " r "<< programDataClient.repetitions << std::endl;
            std::string programName(programDataClient.programName);
            std::string programArgs(programDataClient.programArgs);
            std::string timeoutString(programDataClient.timeoutString);
            try {
                taskList.add(programName, programArgs,
                             timeoutString, programDataClient.repetitions);
            } catch (std::runtime_error &e) {
                return e.what();
            }
            return "Task added successfully";
        }
        case CANCEL:{
            std::cout << "Cancel task cancelTaskID: " << programDataClient.cancelTaskID << std::endl;
            bool status = taskList.remove(programDataClient.cancelTaskID);
            return status ? "Task canceled successfully" : "Task with given ID does not exist!";
        }
        case SHOW:
            std::cout << "Showing task list" << std::endl;
            return taskList.show();
        case QUIT:
            std::cout << "Exiting server!" << std::endl;
            taskList.stop();
            return "Server exit";
        default:
            return "Error";
    }
}
