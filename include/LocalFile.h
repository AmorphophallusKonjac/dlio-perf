#ifndef LOCALFILE_H
#define LOCALFILE_H

#include <File.h>
#include <fstream>
#include <string>

class LocalFile final : public File
{
public:
    bool read(char* buffer, int pos, int size) override;

    bool write(const char* buffer, int pos, int size) override;

    bool open(std::string path, Flag flag) override;

    void close() override;

private:
    std::fstream file_;
};


#endif //LOCALFILE_H
