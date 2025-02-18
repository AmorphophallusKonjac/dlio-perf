#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <string>
#include <memory>

namespace g3
{
    class LogWorker;
}

class LogManager {
public:
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    static LogManager& getInstance(std::string prefix, std::string folder) {
        static LogManager instance(prefix, folder);
        return instance;
    }
private:
    std::unique_ptr<g3::LogWorker> worker;
    LogManager(std::string prefix, std::string folder);
    ~LogManager();
};



#endif //LOGMANAGER_H
