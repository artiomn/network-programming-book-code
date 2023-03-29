#include <memory>


using google::protobuf;

class ExampleSearchService : public SearchService
{
public:
    void Search(protobuf::RpcController* controller,
                const SearchRequest* request,
                SearchResponse* response,
                protobuf::Closure* done)
    {

        if (request->query() == "google")
        {
            response->add_result()->set_url("http://www.google.com");
        }
        else if (request->query() == "protocol buffers")
        {
            response->add_result()->set_url("http://protobuf.googlecode.com");
        }
        done->Run();
   }
};


int main()
{
    // Класс MyRpcServer предоставляется пользователем.
    MyRpcServer server;

    std::unique_ptr<protobuf::Service> service;
    server.ExportOnPort(1234, service.get());
    server.Run();

    return 0;
}

