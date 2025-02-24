#ifndef IOREQUEST_H
#define IOREQUEST_H

#include <FileSystem.h>
#include <string>

class IORequest
{
public:
    enum TaskTy
    {
        READ,
        WRITE,
        CREATE_DIR
    };

    TaskTy ty;
    std::string path;
    long long size;

    IORequest(TaskTy ty, std::string path, long long size);
    IORequest();
    bool empty() const;
    void execute(FileSystem* fs, long long transfer_size, char* buffer) const;
};

#endif //IOREQUEST_H
