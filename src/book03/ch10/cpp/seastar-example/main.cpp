#include <seastar/core/print.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/seastar.hh>
#include <seastar/core/thread.hh>
#include <seastar/http/api_docs.hh>
#include <seastar/http/file_handler.hh>
#include <seastar/http/function_handlers.hh>
#include <seastar/http/handlers.hh>
#include <seastar/http/httpd.hh>
#include <seastar/net/inet_address.hh>
#include <seastar/util/defer.hh>


namespace bpo = boost::program_options;

using namespace seastar;
using namespace httpd;

void set_routes(routes& r)
{
    function_handler* h1 = new function_handler([](const_req req) { return "hello"; });
    function_handler* h2 = new function_handler([](std::unique_ptr<http::request> req)
                                                { return make_ready_future<json::json_return_type>("json-future"); });
    r.add(operation_type::GET, url("/"), h1);
    r.add(operation_type::GET, url("/jf"), h2);
    r.add(operation_type::GET, url("/file").remainder("path"), new directory_handler("/"));
}


int main(int ac, char** av)
{
    app_template app;

    app.add_options()("port", bpo::value<uint16_t>()->default_value(12345), "HTTP Server port");

    return app.run(
        ac, av,
        [&]
        {
            return seastar::async(
                [&]
                {
                    //            seastar_apps_lib::stop_signal stop_signal;
                    seastar::condition_variable cond;

                    seastar::engine().handle_signal(SIGINT, [&cond] { cond.broadcast(); });
                    auto&& config = app.configuration();

                    uint16_t port = config["port"].as<uint16_t>();
                    auto server = new http_server_control();
                    auto rb = make_shared<api_registry_builder>("apps/httpd/");
                    server->start().get();

                    auto stop_server = defer(
                        [&]() noexcept
                        {
                            // This can throw, but won't.
                            std::cout << "Stoppping HTTP server" << std::endl;
                            server->stop().get();
                        });

                    server->set_routes(set_routes).get();
                    server->set_routes([rb](routes& r) { rb->set_api_doc(r); }).get();
                    server->set_routes([rb](routes& r) { rb->register_function(r, "demo", "hello world application"); })
                        .get();
                    server->listen(port).get();

                    std::cout << "Seastar HTTP server listening on port " << port << " ...\n";

                    cond.wait().get();

                    return EXIT_SUCCESS;
                });
        });
}
