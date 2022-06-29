#include "WSListener.hpp"


std::vector<std::string> split_string(std::string str, std::string delimeter)
{
    std::vector<std::string> strings = {};
    size_t pos = 0;

    while ((pos = str.find(delimeter)) != std::string::npos)
    {
        std::string token = str.substr(0, pos);
        // Blank lines also needed as an attribute of the HTTP headers end.
        //if (token.length() > 0)
        strings.push_back(token);
        str.erase(0, pos + delimeter.length());
    }

    if (str.length() > 0) strings.push_back(str);

    return strings;
}


void WSListener::onPing(const WebSocket& socket, const oatpp::String& message)
{
    OATPP_LOGD(TAG, "onPing");
    socket.sendPong(message);
}


void WSListener::onPong(const WebSocket& socket, const oatpp::String& message)
{
    OATPP_LOGD(TAG, "onPong");
}


void WSListener::onClose(const WebSocket& socket, v_uint16 code, const oatpp::String& message)
{
    OATPP_LOGD(TAG, "onClose code=%d", code);
}


void WSListener::readMessage(const WebSocket& socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
{
    if (!size)
    {
        // Message transfer finished
        auto whole_message = message_buffer_.toString();
        message_buffer_.setCurrentPosition(0);

        OATPP_LOGI(TAG, "on message received '%s'", whole_message->c_str());

        oatpp::web::protocol::http::Status error;
        oatpp::web::protocol::http::RequestStartingLine rsl;

        auto strings = std::move(split_string(whole_message, "\n"));
        auto req_line = std::move(split_string(strings[0], " "));

        // HTTP-like: <METHOD> <PATH> <HTTP/<ver>>.
        if (3 == req_line.size())
        {
            auto path = req_line[1];
            std::string parameter;

            for (size_t i = 1; i < strings.size(); ++i)
            {
                // Blank line.
                if (!strings[i].size())
                {
                    if (i < strings.size() - 1) parameter = strings[i + 1];
                    break;
                }
                // headers->append(strings[1]);
            }

            OATPP_LOGD(TAG, "Path: %s", path.c_str());
            OATPP_LOGD(TAG, "Parameter: %s", parameter.c_str());
        }
        // Send message in reply.
        socket.sendOneFrameText("200 OK");
    }
    else if (size > 0)
    { // message frame received
        message_buffer_.writeSimple(data, size);
    }
}
