#ifndef FILE_H
#define FILE_H

#include <string>

class File
{
public:
    enum Flag
    {
        READ = 1,
        WRITE = 1 << 1
    };

    virtual bool read(char* buffer, long long pos, long long size) = 0;

    virtual bool write(const char* buffer, long long pos, long long size) = 0;

    virtual bool writeWholeFile(long long size, long long transfer_size) = 0;

    virtual bool open(std::string path, Flag flag) = 0;

    virtual void close() = 0;

    virtual ~File() = default;

protected:
    std::string path_;
    long long pos_ = 0;
};


#endif //FILE_H
