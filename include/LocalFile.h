#ifndef LOCALFILE_H
#define LOCALFILE_H

#include <File.h>
#include <fstream>
#include <string>

class LocalFile final : public File
{
public:
    bool read(char* buffer, long long pos, long long size) override;

    bool write(const char* buffer, long long pos, long long size) override;

    bool writeWholeFile(long long size, long long transfer_size) override;

    bool readWholeFile(long long size, long long transfer_size) override;

    bool open(std::string path, Flag flag) override;

    void close() override;

    ~LocalFile() override = default;

private:
    std::fstream file_;
};


#endif //LOCALFILE_H
