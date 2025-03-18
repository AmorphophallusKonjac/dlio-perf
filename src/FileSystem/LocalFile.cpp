#include "LocalFile.h"

#include <spdlog/spdlog.h>

bool LocalFile::read(char* buffer, const long long pos, const long long size) {
    try {
        if (pos != pos_) {
            file_.seekg(pos);
            pos_ = pos;
        }
        file_.read(buffer, size);
        pos_ = pos + size;
        return true;
    } catch (const std::exception& e) {
        spdlog::error("read {} fail, {}", path_, e.what());
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
        file_.flush();
        pos_ = pos + size;
        return true;
    } catch (const std::exception& e) {
        spdlog::error("write {} fail, {}", path_, e.what());
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
        spdlog::error("write {} fail, {}", path_, e.what());
        return false;
    }
}

bool LocalFile::readWholeFile(long long size, long long transfer_size) {
    try {
        const auto buffer = new char[transfer_size];
        if (pos_ != 0) {
            file_.seekg(0);
            pos_ = 0;
        }
        while (pos_ < size) {
            if (pos_ + transfer_size > size)
                read(buffer, pos_, size - pos_);
            else
                read(buffer, pos_, transfer_size);
        }
        delete[] buffer;
        return true;
    } catch (const std::exception& e) {
        spdlog::error("read {} fail, {}", path_, e.what());
        return false;
    }
}

bool LocalFile::open(std::string path, Flag flag) {
    try {
        path_ = path;
        switch (flag) {
            case READ:
                file_.open(path, std::ios::in);
                pos_ = file_.tellg();
                break;
            case WRITE:
                file_.open(path, std::ios::out);
                pos_ = file_.tellp();
                break;
        }
        return true;
    } catch (const std::exception& e) {
        spdlog::error("open {} fail, {}", path_, e.what());
        return false;
    }
}

void LocalFile::close() {
    file_.close();
}



