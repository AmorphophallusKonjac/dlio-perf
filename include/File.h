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

    virtual bool read(char* buffer, int pos, int size) = 0;

    virtual bool write(const char* buffer, int pos, int size) = 0;

    virtual bool open(std::string path, Flag flag) = 0;

    virtual void close() = 0;

    virtual ~File();

protected:
    std::string path_;
    int pos_ = 0;
};


#endif //FILE_H
