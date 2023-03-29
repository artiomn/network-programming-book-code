#include <memory>

using google::protobuf;

std::unique_ptr<protobuf::RpcChannel> channel;
std::unique_ptr<protobuf::RpcController> controller;

std::unique_ptr<SearchService> service;
SearchRequest request;
SearchResponse response;


int main()
{
    channel = std::make_unique<MyRpcChannel>("localhost:1234");
    controller = std::make_unique<MyRpcController>();

    service = std::make_unique<SearchService::Stub>(channel);

    request.set_query("protocol buffers");

    service->Search(controller, request, response,
                    protobuf::NewCallback(&Done));
}

