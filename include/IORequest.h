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
    long long transfer_size;
    IORequest(TaskTy ty, std::string path, long long size, long long pos, long long transfer_size, FileSystem* fs);
    IORequest(TaskTy ty, std::string path, long long transfer_size, FileSystem* fs);
    IORequest();
    bool empty() const;
    void execute() const;
    void addIOReq(long long pos, long size);

private:
    FileSystem* fs;
    std::vector<long long> size_;
    std::vector<long long> pos_;
};

#endif //IOREQUEST_H
