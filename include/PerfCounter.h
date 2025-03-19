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

    void addOperation(OperationTy ty, std::chrono::steady_clock::time_point start_time,
                      std::chrono::steady_clock::time_point end_time, long long size = 0);
    void setRefTimePoint(std::chrono::steady_clock::time_point start_time_point,
                         std::chrono::steady_clock::time_point end_time_point);
    void preprocess();
    YAML::Node perfThisRank();
    YAML::Node perfAllRank();

private:
    PerfCounter() = default;
    YAML::Node getPerfResult(std::vector<OperationRefInfo>& ops);
    static void calcQpsAndBw(std::vector<OperationRefInfo>& ops, QPS_perf& qps, BW_perf& bw);
    template <typename T>
    std::vector<std::pair<double, T>> getPercentiles(std::vector<T>& vec, std::vector<double>& per);
    template <typename T>
    YAML::Node percentilesToYaml(const std::vector<std::pair<double, T>>& pairs);

    std::vector<OperationInfo> ops_;
    std::vector<OperationRefInfo> ref_ops_;
    std::vector<OperationRefInfo> all_rank_ref_ops;
    std::mutex ops_mutex_;
    std::vector<double> latPercentiles = {
        1.00, 5.00, 10.00, 20.00, 30.00, 40.00, 50.00,
        60.00, 70.00, 80.00, 90.00, 95.00, 99.00, 99.90, 99.99
    };
    std::vector<double> graPercentiles = {10.00, 20.00, 30.00, 40.00, 50.00, 60.00, 70.00, 80.00, 90.00};
    std::chrono::steady_clock::time_point start_time_point_;
    std::chrono::steady_clock::time_point end_time_point_;
};

#endif //PERFCOUNTER_H
