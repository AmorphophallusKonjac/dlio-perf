#ifndef PERFCOUNTER_H
#define PERFCOUNTER_H

#include <mutex>
#include <OperationInfo.h>

#include <vector>
#include <PerfResult.h>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/node/node.h>


class PerfCounter
{
public:
    static PerfCounter& getInstance()
    {
        static PerfCounter instance;
        return instance;
    }

    void addOperation(const OperationInfo& op);
    void addOperation(OperationInfo::OperationTy ty, std::chrono::steady_clock::time_point start_time,
                      std::chrono::steady_clock::time_point end_time);
    void addOperation(OperationInfo::OperationTy ty, std::chrono::steady_clock::time_point start_time,
                      std::chrono::steady_clock::time_point end_time, long long size);
    YAML::Node getPerfResult();

private:
    PerfCounter() = default;
    void calcPerfResult(std::vector<OperationInfo>& ops, QPS_perf& qps, BW_perf& bw);
    std::vector<OperationInfo> read_ops_;
    std::vector<OperationInfo> write_ops_;
    std::vector<OperationInfo> open_ops_;
    QPS_perf read_qps_, write_qps_, open_qps_;
    BW_perf read_bw_, write_bw_;
    std::mutex ops_mutex_;
};


#endif //PERFCOUNTER_H
