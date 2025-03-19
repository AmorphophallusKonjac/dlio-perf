#ifndef OPERATIONINFO_H
#define OPERATIONINFO_H

#include<chrono>

enum OperationTy
{
    READ,
    WRITE,
    OPEN
};

struct OperationInfo
{
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    std::chrono::nanoseconds latency;
    OperationTy ty;
    long long size;
};

struct OperationRefInfo
{
    long long ref_start_time; // second
    long long ref_end_time; // second
    long long latency; // microsecond
    long long size;
    long long ty;
};


#endif //OPERATIONINFO_H
