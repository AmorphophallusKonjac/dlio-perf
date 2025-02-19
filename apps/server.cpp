#include <iostream>
#include <rest_rpc.hpp>
#include <chrono>
using namespace rest_rpc;
using namespace rest_rpc::rpc_service;

/*服务函数第一个参数必须为 rpc_conn，然后才是实现功能需要的参数（为可变参数，数量可变，也可以没有*/
std::string hello(rpc_conn conn, std::string name){
	/*可以为 void 返回类型，代表调用后不给远程客户端返回消息*/
    return ("Hello " + name); /*返回给远程客户端的内容*/
}


int main(){
    rpc_server server(9000, 6);

    /*func_greet 为服务名，远程调用通过服务名确定调用函数*/
    /*hello 为函数，绑定当前服务调用哪个函数*/
    server.register_handler("func_greet", hello);

    server.run();//启动服务端

    std::cout << "server here" << std::endl;

    return EXIT_SUCCESS;
}
