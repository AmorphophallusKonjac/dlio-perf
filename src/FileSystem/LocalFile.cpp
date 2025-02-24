#include "LocalFile.h"

#include <asio/registered_buffer.hpp>
#include <g3log/g3log.hpp>

bool LocalFile::read(char* buffer, const long long pos, const long long size) {
    try {
        if (pos != pos_) {
            file_.seekp(pos);
            pos_ = pos;
        }
        file_.read(buffer, size);
        pos_ = pos + size;
        return true;
    } catch (const std::exception& e) {
        LOGF(FATAL, "read %s fail, %s", path_.c_str(), e.what());
        return false;
    }
}

bool LocalFile::write(const char* buffer, long long pos, long long size) {
    try {
        if (pos != pos_) {
            file_.seekp(pos);
            pos_ = pos;
        }
        file_.write(buffer, size);
        pos_ = pos + size;
        return true;
    } catch (const std::exception& e) {
        LOGF(FATAL, "write %s fail, %s", path_.c_str(), e.what());
        return false;
    }
}

bool LocalFile::writeWholeFile(long long size, long long transfer_size) {
    try {
        const auto buffer = new char[transfer_size];
        if (pos_ != 0) {
            file_.seekp(0);
            pos_ = 0;
        }
        while (pos_ < size) {
            if (pos_ + transfer_size > size)
                write(buffer, pos_, size - pos_);
            else
                write(buffer, pos_, transfer_size);
        }
        delete[] buffer;
        return true;
    } catch (const std::exception& e) {
        LOGF(FATAL, "write %s fail, %s", path_.c_str(), e.what());
        return false;
    }
}

bool LocalFile::open(std::string path, Flag flag) {
    try {
        path_ = path;
        switch (flag) {
            case READ:
                file_.open(path, std::ios::in);
                break;
            case WRITE:
                file_.open(path, std::ios::out);
                break;
        }
        return true;
    } catch (const std::exception& e) {
        LOGF(FATAL, "open %s fail, %s", path_.c_str(), e.what());
        return false;
    }
}

void LocalFile::close() {
    file_.close();
}



