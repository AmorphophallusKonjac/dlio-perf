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
    static void calcPerfResult(std::vector<OperationInfo>& ops, QPS_perf& qps, BW_perf& bw);
    template <typename T>
    std::vector<std::pair<double, T>> getPercentiles(std::vector<T>& vec, std::vector<double>& per);
    template <typename T>
    YAML::Node percentilesToYaml(const std::vector<std::pair<double, T>>& pairs);

    std::vector<OperationInfo> read_ops_;
    std::vector<long long> lat_read_ops_;
    std::vector<long long> gra_read_ops_;
    std::vector<OperationInfo> write_ops_;
    std::vector<long long> lat_write_ops_;
    std::vector<long long> gra_write_ops_;
    std::vector<OperationInfo> open_ops_;
    std::vector<long long> lat_open_ops_;
    QPS_perf read_qps_, write_qps_, open_qps_;
    BW_perf read_bw_, write_bw_;
    std::mutex ops_mutex_;
    std::vector<double> latPercentiles = {
        1.00, 5.00, 10.00, 20.00, 30.00, 40.00, 50.00,
        60.00, 70.00, 80.00, 90.00, 95.00, 99.00, 99.90, 99.99
    };
    std::vector<double> graPercentiles = {10.00, 20.00, 30.00, 40.00, 50.00, 60.00, 70.00, 80.00, 90.00};
};

#endif //PERFCOUNTER_H
