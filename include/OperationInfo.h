#ifndef OPERATIONINFO_H
#define OPERATIONINFO_H

#include<chrono>


class OperationInfo
{
public:
    enum OperationTy
    {
        READ,
        WRITE,
        OPEN
    };

    OperationInfo(OperationTy ty, std::chrono::steady_clock::time_point start_time,
                  std::chrono::steady_clock::time_point end_time);
    OperationInfo(OperationTy ty, std::chrono::steady_clock::time_point start_time,
                  std::chrono::steady_clock::time_point end_time, long long size);
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    std::chrono::nanoseconds latency{};
    OperationTy ty;
    long long size;
};


#endif //OPERATIONINFO_H
