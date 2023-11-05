#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>
#include <search_service.grpc.pb.h>
#include <search_service.pb.h>

#include <iostream>
#include <memory>


class ExampleSearchServiceImpl final : public SearchService::Service
{
public:
    grpc::Status Search(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) override
    {
        const auto query = request->query();
        std::cout << "Query = " << query << std::endl;

        auto result = response->add_result();

        if ("google" == query)
        {
            result->set_title("Google");
            result->set_url("http://www.google.com");
        }
        else if ("yandex" == request->query())
        {
            result->set_title("Yandex");
            result->set_url("http://www.yandex.ru");
        }

        return grpc::Status::OK;
    }
};


int main()
{
    grpc::ServerBuilder server_builder;

    server_builder.AddListeningPort("0.0.0.0:12345", grpc::InsecureServerCredentials());

    ExampleSearchServiceImpl service;

    server_builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(server_builder.BuildAndStart());
    server->Wait();

    return EXIT_SUCCESS;
}
