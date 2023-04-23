#include <httpserver.hpp>


using namespace httpserver;

// Обработчик доступа к ресурсу.
class hello_world_resource : public http_resource
{
public:
    std::shared_ptr<http_response> render(const http_request&)
    {
        return std::make_shared<http_response>(string_response("Hello, World!"));
    }
};


int main(int argc, const char* const argv[])
{
    // Создать объект сервера.
    webserver ws = create_webserver(12345);

    hello_world_resource hwr;
    // Зарегистрировать обработчик доступа к ресурсу https://<server>/hello
    ws.register_resource("/hello", &hwr);
    // Запустить сервер.
    ws.start(true);

    return EXIT_SUCCESS;
}
