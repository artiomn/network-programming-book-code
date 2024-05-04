#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>
#include <search_service.grpc.pb.h>
#include <search_service.pb.h>

#include <iostream>
#include <memory>


int main()
{
    auto channel = grpc::CreateChannel("localhost:12345", grpc::InsecureChannelCredentials());
    std::unique_ptr<SearchService::Stub> client_stub{SearchService::NewStub(channel)};

    grpc::ClientContext context;
    SearchResponse response;
    SearchRequest request;

    request.set_query("google");

    auto status = client_stub->Search(&context, request, &response);

    if (!status.ok())
    {
        std::cerr << "RPC call error: [" << status.error_message() << "]!" << std::endl;
        return EXIT_FAILURE;
    }

    for (const auto &result : response.result())
    {
        if (result.has_url()) std::cout << result.url() << std::endl;
    }
    return EXIT_SUCCESS;
}
