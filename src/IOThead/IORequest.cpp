#include "IORequest.h"

#include <asio/registered_buffer.hpp>
#include <g3log/g3log.hpp>

IORequest::IORequest(const TaskTy ty, std::string path, const long long size,
                     const long long pos) :
    ty(ty), path(std::move(path)), size(size), pos(pos) {
}

IORequest::IORequest() : ty(READ), size(0), pos(-1) {
}

bool IORequest::empty() const {
    return path.empty();
}

void IORequest::execute(FileSystem* fs, const long long transfer_size,
                        char* buffer) const {
    const auto file = fs->getFileDescriptor();
    if (pos == -1) {
        switch (ty) {
            case READ:
                file->open(path, File::Flag::READ);
                for (long long i = 0; i < size; i = i + transfer_size) {
                    if (i + transfer_size > size)
                        file->read(buffer, i, size - i);
                    else
                        file->read(buffer, i, transfer_size);
                }
                file->close();
                break;
            case WRITE:
                file->open(path, File::Flag::WRITE);
                for (long long i = 0; i < size; i = i + transfer_size) {
                    if (i + transfer_size > size)
                        file->write(buffer, i, size - i);
                    else
                        file->write(buffer, i, transfer_size);
                }
                file->close();
                break;
            case CREATE_DIR:
                fs->createDir(path);
                break;
        }
    } else {
        switch (ty) {
            case READ:
                file->open(path, File::Flag::READ);
                file->read(buffer, pos, size);
                file->close();
                break;
            case WRITE:
                file->open(path, File::Flag::WRITE);
                file->write(buffer, pos, size);
                file->close();
                break;
            case CREATE_DIR:
                throw std::runtime_error("Error IORequest type");
                break;
        }
    }
}