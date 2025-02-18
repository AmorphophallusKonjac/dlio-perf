#include <iostream>
#include <rest_rpc.hpp>
#include <chrono>
using namespace rest_rpc;
using namespace rest_rpc::rpc_service;

int main(){
    /* rest_rpc 在遇到错误（调用服务传入参数和远程服务需要参数不一致、连接失败等）时会抛出异常*/
    try{

        /*建立连接*/
        rpc_client client("127.0.0.1", 9000);// IP 地址，端口号
        /*设定超时 5s（不填默认为 3s），connect 超时返回 false，成功返回 true*/
        bool has_connected = client.connect(5);
        /*没有建立连接则退出程序*/
        if (!has_connected) {
            std::cout << "connect timeout" << std::endl;
            exit(-1);
        }

        /*调用远程服务，返回欢迎信息*/
        std::string result = client.call<std::string>("func_greet", "HG");// func_greet 为事先注册好的服务名，需要一个 name 参数，这里为 Hello Github 的缩写 HG
        std::cout << result << std::endl;

    }
    /*遇到连接错误、调用服务时参数不对等情况会抛出异常*/
    catch (const std::exception & e) {
        std::cout << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}