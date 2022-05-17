#include <iostream>
#include <unistd.h>
#include <cstring>
#include "ConsoleAPI.h"
#include "Logger.h"

int main(int argc, char **argv) {
    ConsoleAPI consoleAPI;
    if (argc == 1) {
        Logger logger("../logs.txt");
        std::cout << "Server started! at " << getpid() << std::endl;
        struct mq_attr attributes{};

        attributes.mq_maxmsg = 10;
        attributes.mq_msgsize = sizeof(ProgramDataClient);
        attributes.mq_flags = 0;

        mqd_t mq_queries_from_clients = mq_open("/server_queue", O_CREAT | O_RDONLY, 0600, &attributes);
        if (mq_queries_from_clients < 0) {
            std::cout << "Error in creating server queue" << std::endl;
            return -1;
        }
        ProgramDataClient programData;
        size_t ret;
        while (true) {
            ret = mq_receive(mq_queries_from_clients, (char *) &programData, sizeof(ProgramDataClient), nullptr);
            if (ret < 0) {
                std::cout << "Error in receiving message" << std::endl;
                return -1;
            }
            std::string response = consoleAPI.parseCommand(programData);

            ServerResponse serverResponse{};
            memcpy(serverResponse.response, response.c_str(), response.length());
            serverResponse.status = response != "Error" ? 0 : -1;

            mqd_t mq_reply_to_client = mq_open(programData.queueName, O_WRONLY);
            if (mq_reply_to_client < 0) {
                std::cout << "Error in creating client queue" << std::endl;
                return -1;
            }
            ret = mq_send(mq_reply_to_client, (const char *) &serverResponse, sizeof(ServerResponse), 0);
            if (ret < 0) {
                std::cout << "Error in sending message" << std::endl;
                return -1;
            }
            mq_close(mq_reply_to_client);
            if (programData.action == ADD) {
                logger.log(INFO, MAXIMUM_PRIORITY, "Task added: ", programData.programName);
            }
            else if (programData.action == CANCEL) {
                logger.log(INFO, MAXIMUM_PRIORITY, "Task cancelled with id ", programData.cancelTaskID);
            }
            else if (programData.action == SHOW) {
                logger.log(INFO, MINIMUM_PRIORITY, "Task list shown");
            }
            else if (programData.action == HELP) {
                logger.log(INFO, MINIMUM_PRIORITY, "Help shown");
            }
            else if (programData.action == QUIT) {
                logger.log(INFO, MAXIMUM_PRIORITY, "Exiting server");
                break;
            } else {
                logger.log(INFO, BASIC_PRIORITY, "Error parsing command");
            }

        }
        mq_close(mq_queries_from_clients);
        mq_unlink("/server_queue");
        return 0;
    }

    ProgramDataClient programDataClient = ConsoleAPI::getAction(argc - 1, ++argv);
    if (programDataClient.action == UNDEFINED) {
        std::cout << "Error in arguments!" << std::endl;
        return 0;
    }
    else if (programDataClient.action == HELP) {
        ConsoleAPI::printHelp();
        return 0;
    }

    mqd_t mq_queries_to_server;
    int trails = 0;
    do {
        mq_queries_to_server = mq_open("/server_queue", O_WRONLY);
        sleep(1);
        if (++trails > 5) {
            std::cout << "Error in opening server queue" << std::endl;
            return -1;
        }
    } while (mq_queries_to_server == -1);

    struct mq_attr attr{};

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(ServerResponse);
    attr.mq_flags = 0;

    mqd_t mq_reply_from_server = mq_open(programDataClient.queueName, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq_reply_from_server < 0) {
        std::cout << "Error in creating client queue" << std::endl;
        return -1;
    }
    size_t ret = mq_send (mq_queries_to_server, (const char*)&programDataClient, sizeof(ProgramDataClient), 0);
    if (ret < 0) {
        std::cout << "Error in sending message" << std::endl;
        return -1;
    }
    ServerResponse reply;
    mq_receive (mq_reply_from_server, (char*)&reply, sizeof(ServerResponse), nullptr);
    if (ret < 0) {
        std::cout << "Error in receiving message" << std::endl;
        return -1;
    }
    std::cout << reply.response << std::endl;
    std::cout << "Status: " << reply.status << std::endl;
    mq_close(mq_queries_to_server);
    mq_close(mq_reply_from_server);
    return 0;
}