#include "device.hpp"
#include "serial_port.hpp"
#include "WSListener.hpp"

#include <oatpp-websocket/WebSocket.hpp>
#include <oatpp-websocket/Connector.hpp>

#include <oatpp/network/tcp/client/ConnectionProvider.hpp>

#include <oatpp/web/client/HttpRequestExecutor.hpp>
#include <oatpp/network/tcp/client/ConnectionProvider.hpp>

#include <oatpp/parser/json/mapping/ObjectMapper.hpp>

#include <csignal>
#include <fstream>
#include <string>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>


namespace
{
const char* TAG = "driver-console";
const char* server_api_base = "artiomn/robotic_arm_server/1.0.0/";

std::function<void(int)> shutdown_handler;
void signal_handler(int signal) { shutdown_handler(signal); }

}


std::shared_ptr<oatpp::web::client::RequestExecutor> create_oatpp_executor(const std::string &host, unsigned short port)
{
    OATPP_LOGD(TAG, "Using Oat++ native HttpRequestExecutor.");
    auto connection_provider = oatpp::network::tcp::client::ConnectionProvider::createShared({host, port});

    return oatpp::web::client::HttpRequestExecutor::createShared(connection_provider);
}


std::string get_uuid(const std::string &serial_port)
{
    std::ifstream id_file("/sys/class/dmi/id/product_uuid");
    std::string machine_uid;

    if (id_file)
    {
        id_file >> machine_uid;
    }
    else
    {
        OATPP_LOGE(TAG, "Cant read machine unique identificator!");
        throw std::logic_error("Can't read machine UID!");
    }
    id_file.close();

    OATPP_LOGD(TAG, "Machine UUID = %s", machine_uid.c_str());

    // Generate unique identifier for the machine and port.
    auto device_uuid = boost::lexical_cast<std::string>(boost::uuids::name_generator(boost::uuids::ns::oid())(machine_uid + serial_port));
    OATPP_LOGI(TAG, "Device UUID = %s", device_uuid.c_str());

    return device_uuid;
}


int run(const std::string &host, unsigned short port, const std::string &serial_port)
{
    OATPP_LOGI(TAG, "Application Started");

    const auto device_uuid = std::move(get_uuid(serial_port));

    // Create ObjectMapper for serialization of DTOs.
    auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

    // Create RequestExecutor which will execute ApiClient's requests.
    auto request_executor = create_oatpp_executor(host, port);

    // DemoApiClient uses DemoRequestExecutor and json::mapping::ObjectMapper.
    // ObjectMapper passed here is used for serialization of outgoing DTOs.
    auto client = TgArmBotApiClient::createShared(request_executor, object_mapper);

    auto registration_info = DeviceRegistrationInfoDTO::createShared();

    OATPP_LOGI(TAG, "Device registration started");
    registration_info->id = device_uuid;
    registration_info->name = "Robotic Arm Device";
    client->register_device(server_api_base, registration_info);
    OATPP_LOGI(TAG, "Device was registered");

    OATPP_LOGI(TAG, "Creating Websocket connection...");
    auto connectionProvider = oatpp::network::tcp::client::ConnectionProvider::createShared({host, port});
    auto connector = oatpp::websocket::Connector::createShared(connectionProvider);
    auto connection = connector->connect(server_api_base);

    OATPP_LOGI(TAG, "Connected");

    auto socket = oatpp::websocket::WebSocket::createShared(connection, true /* maskOutgoingMessages must be true for clients */);

    socket->setListener(std::make_shared<WSListener>(SerialPort(serial_port)));

    OATPP_LOGD(TAG, "Sending device UID...");
    socket->sendOneFrameText("UID " + device_uuid);

    socket->listen();
    OATPP_LOGD(TAG, "SOCKET CLOSED!!!");

    return EXIT_SUCCESS;
}


int main(int argc, const char * const argv[])
{
    oatpp::base::Environment::init();

    if (argc < 4)
    {
        OATPP_LOGE(TAG, " <host> <port> <serial port>");
        return EXIT_FAILURE;
    }

    const auto host = argv[1];
    const auto port = std::stoi(argv[2]);
    const auto serial_port = argv[3];

    shutdown_handler = [&host, &port, &serial_port](int s)
    {
        OATPP_LOGD(TAG, "SIGINT got");
        auto object_mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

        auto request_executor = create_oatpp_executor(host, port);
        auto client = TgArmBotApiClient::createShared(request_executor, object_mapper);
        auto registration_info = DeviceRegistrationInfoDTO::createShared();

        OATPP_LOGI(TAG, "Device was unregistered");
        client->unregister_device(server_api_base, std::move(get_uuid(serial_port)));

        exit(EXIT_SUCCESS);
    };

    if (SIG_ERR == signal(SIGINT, signal_handler))
    {
        OATPP_LOGE(TAG, "signal() failed");
        return EXIT_FAILURE;
    }

    auto result = run(host, port, serial_port);
    oatpp::base::Environment::destroy();

    return result;
}

