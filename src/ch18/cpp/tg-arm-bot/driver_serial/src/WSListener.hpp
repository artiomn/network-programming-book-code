#pragma once

#include <oatpp-websocket/ConnectionHandler.hpp>
#include <oatpp-websocket/WebSocket.hpp>
#include <oatpp/web/protocol/http/Http.hpp>

#include "serial_port.hpp"


// WebSocket listener listens on incoming WebSocket events.
class WSListener : public oatpp::websocket::WebSocket::Listener
{
public:
    WSListener(SerialPort &&sp) : sp_(std::move(sp))
    {}

    // Called on "ping" frame.
    void onPing(const WebSocket& socket, const oatpp::String& message) override;

    // Called on "pong" frame
    void onPong(const WebSocket& socket, const oatpp::String& message) override;

    // Called on "close" frame
    void onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message) override;

    // Called on each message frame. After the last message will be called once-again with size == 0 to designate end of the message.
    void readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;

private:
    static constexpr const char* TAG = "Client_WSListener";

private:
    std::string state_command_process(const std::string& method, const std::string& parameter);
    std::string shoulder_command_process(const std::string& method, const std::string& action, const std::string& parameter);
    std::string forearm_command_process(const std::string& method, const std::string& action, const std::string& parameter);
    std::string manipulator_command_process(const std::string& method, const std::string& action, const std::string& parameter);

private:
    // Buffer for messages. Needed for multi-frame messages.
    oatpp::data::stream::BufferOutputStream message_buffer_;
    SerialPort sp_;
};

