#include "web_socket.h"

#include <cassert>
#include <cstring>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <system_error>
#include <utility>

extern "C"
{
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
}


using namespace std::chrono;


namespace
{
static std::map<std::string, std::shared_ptr<restbed::WebSocket>> sockets_ = {};
static std::map<std::string, std::string> devices_to_ws_keys = {};

const std::string uid_prefix = "UID ";

static WsMessageHandler ws_on_message_handler;

}


std::string base64_encode(const unsigned char* input, int length)
{
    BIO* bmem, *b64;
    BUF_MEM* bptr;

    b64 = BIO_new(BIO_f_base64());
    bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, input, length);
    BIO_flush(b64);
    BIO_get_mem_ptr(b64, &bptr);

    std::string buff;
    buff.resize(bptr->length);
    std::copy(bptr->data, bptr->data + bptr->length - 1, buff.data());
    buff[bptr->length - 1] = 0;

    BIO_free_all(b64);

    return buff.data();
}


std::multimap<std::string, std::string> build_websocket_handshake_response_headers(
    const std::shared_ptr<const restbed::Request>& request)
{
    auto key = request->get_header("Sec-WebSocket-Key");
    key.append("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

    restbed::Byte hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(key.data()), key.length(), hash);

    std::multimap<std::string, std::string> headers;
    headers.insert(std::make_pair("Connection", "Upgrade"));
    headers.insert(std::make_pair("Upgrade", "websocket"));
    headers.insert(std::make_pair("Sec-WebSocket-Accept", base64_encode( hash, SHA_DIGEST_LENGTH)));

    return headers;
}


void ws_ping_handler()
{
    for (auto entry : sockets_)
    {
        auto key = entry.first;
        auto socket = entry.second;

        if (socket->is_open())
        {
            socket->send(restbed::WebSocketMessage::PING_FRAME);
        }
        else
        {
            socket->close();
        }
    }
}


void ws_close_handler(const std::shared_ptr<restbed::WebSocket> socket)
{
    socket->set_close_handler(nullptr);
    socket->set_error_handler(nullptr);
    socket->set_message_handler(nullptr);

    if (socket->is_open())
    {
        auto response = std::make_shared<restbed::WebSocketMessage>(restbed::WebSocketMessage::CONNECTION_CLOSE_FRAME, restbed::Bytes({10, 00}));
        socket->send(response);
    }

    const auto key = socket->get_key();
    std::cout
        << "Closed connection to "
        << key.data()
        << std::endl;

    sockets_.erase(key);
}


void ws_error_handler(const std::shared_ptr<restbed::WebSocket> socket,
                      const std::error_code error)
{
    socket->set_close_handler(nullptr);
    socket->set_error_handler(nullptr);
    socket->set_message_handler(nullptr);

    const auto key = socket->get_key();
    std::cerr
        << "WebSocket Errored '"
        <<  error.message().data()
        << "' for "
        << key.data()
        << std::endl;
    sockets_.erase(key);
}


void ws_message_handler(const std::shared_ptr<restbed::WebSocket> source,
                        const std::shared_ptr<restbed::WebSocketMessage> message)
{
    const auto opcode = message->get_opcode();

    if (restbed::WebSocketMessage::PING_FRAME == opcode)
    {
        auto response = std::make_shared<restbed::WebSocketMessage>(restbed::WebSocketMessage::PONG_FRAME, message->get_data());
        source->send(response);
    }
    else if (restbed::WebSocketMessage::PONG_FRAME == opcode)
    {
        // Ignore PONG_FRAME.
        return;
    }
    else if (restbed::WebSocketMessage::CONNECTION_CLOSE_FRAME  == opcode)
    {
        source->close();
    }
    else if (restbed::WebSocketMessage::BINARY_FRAME == opcode)
    {
        // We don't support binary data.
        auto response = std::make_shared<restbed::WebSocketMessage>(
            restbed::WebSocketMessage::CONNECTION_CLOSE_FRAME, restbed::Bytes({10, 03}));
        source->send(response);
    }
    else if (restbed::WebSocketMessage::TEXT_FRAME == opcode)
    {
        if (!message->get_data().size())
        {
            std::cout << "Empty message" << std::endl;
            return;
        }

        auto data = message->get_data();
        std::string client_text(data.begin(), data.end());

        assert(message->get_data().size() == client_text.size());

        const auto key = source->get_key();
        const auto msg = restbed::String::format("Received message '%.*s' from %s\n",
            client_text.size(), client_text.c_str(), key.data());

        std::cout
            << msg.data()
            << std::endl;

        if (ws_on_message_handler) ws_on_message_handler(key.data(), msg.data());

        if (0 == client_text.rfind(uid_prefix))
        {
            const auto client_uid = client_text.substr(uid_prefix.length());
            std::cout
                << client_uid
                << " -> "
                << key.data()
                << std::endl;
            devices_to_ws_keys.emplace(std::make_pair(client_uid, key.data()));
        }
    }
}


void ws_get_method_handler(const std::shared_ptr<restbed::Session> session)
{
    const auto request = session->get_request();
    const auto connection_header = request->get_header("connection", restbed::String::lowercase);

    if (connection_header.find("upgrade") not_eq std::string::npos)
    {
        if ("websocket" == request->get_header("upgrade", restbed::String::lowercase))
        {
            const auto headers = build_websocket_handshake_response_headers(request);

            session->upgrade(restbed::SWITCHING_PROTOCOLS, headers, [](const std::shared_ptr<restbed::WebSocket> socket)
            {
                if (socket->is_open())
                {
                    socket->set_close_handler(ws_close_handler);
                    socket->set_error_handler(ws_error_handler);
                    socket->set_message_handler(ws_message_handler);

                    socket->send("Robotic Arm Ws Channel",
                        [](const std::shared_ptr<restbed::WebSocket> socket)
                    {
                        const auto key = socket->get_key();

                        sockets_.insert(std::make_pair(key, socket));
                        std::cout
                            << "Sent welcome message to "
                            << key.data()
                            << std::endl;
                    });
                }
                else
                {
                    std::cerr
                        << "WebSocket Negotiation Failed: Client closed connection."
                        << std::endl;
                }
            });

            return;
        }
    }

    session->close(restbed::BAD_REQUEST);
}


void ws_create(std::shared_ptr<restbed::Service> service, const std::string &url, long ping_timeout)
{
    auto resource = std::make_shared<restbed::Resource>();

    std::cout
        << "Websocket path: "
        << url
        << std::endl;
    resource->set_path(url);
    resource->set_method_handler("GET", ws_get_method_handler);
    service->publish(resource);
    service->schedule(ws_ping_handler, milliseconds(ping_timeout));
}


void ws_send_command(const std::string &device_id, const std::string& command)
{
    auto destination = sockets_[devices_to_ws_keys[device_id]];
    destination->send(command);
}

