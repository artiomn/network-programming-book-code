#include "WSListener.hpp"

#include <cassert>


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
            const auto& method = req_line[0];
            const auto& path = req_line[1];
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

            OATPP_LOGD(TAG, "Method: %s", method.c_str());
            OATPP_LOGD(TAG, "Path: %s", path.c_str());
            OATPP_LOGD(TAG, "Parameter: %s", parameter.c_str());

            auto path_components = std::move(split_string(path, "/"));

            // Skip first "/" if exists.
            const int start_index = path_components[0].size() ? 0 : 1;

            if ((path_components.size() < (start_index + 2)) || ("device" != path_components[start_index])) return;

            const auto& d_part = path_components[start_index + 1];

            OATPP_LOGD(TAG, "Device part: %s", d_part.c_str());

            std::string command;

            if ("state" == d_part)
            {
                command = std::move(state_command_process(method, parameter));
            }
            else if ("shoulder" == d_part)
            {
                assert(path_components.size() > (start_index + 2));
                command = std::move(shoulder_command_process(method, path_components[start_index + 2], parameter));
            }
            else if ("forearm" == d_part)
            {
                assert(path_components.size() > (start_index + 2));
                command = std::move(forearm_command_process(method, path_components[start_index + 2], parameter));
            }
            else if ("manipulator" == d_part)
            {
                assert(path_components.size() > (start_index + 2));
                command = std::move(manipulator_command_process(method, path_components[start_index + 2], parameter));
            }
            else
            {
                assert(false);
            }

            OATPP_LOGD(TAG, "Device command: %s", command.c_str());

            if (!command.size())
            {
                socket.sendOneFrameText("501 Not Implemented");
                return;
            }

            sp_.write(command);
        }
        // Send message in reply.
        socket.sendOneFrameText("200 OK");
    }
    else if (size > 0)
    { // message frame received
        message_buffer_.writeSimple(data, size);
    }
}


std::string WSListener::state_command_process(const std::string& method, const std::string& parameter)
{
    OATPP_LOGD(TAG, "State processor: method = %s, parameter = %s", method.c_str(), parameter.c_str());

    // Only changing device state was implemented.
    if (method != "PUT") return "";

    std::string command = "!";

    if ("rebooting" == parameter) command += "reboot!";
    else return "";

    return command;
}


std::string WSListener::shoulder_command_process(const std::string& method, const std::string& action, const std::string& parameter)
{
    OATPP_LOGD(TAG, "Shoulder processor: method = %s, action = %s, parameter = %s",
               method.c_str(), action.c_str(), parameter.c_str());

    // Only changing device state was implemented.
    if (method != "PUT") return "";
    std::string command("!");

    if ("rotate_angle" == action) command += "r";
    else if ("lift_angle" == action) command += "l";
    else return "";

    command += "s" + parameter + "!";

    return command;
}


std::string WSListener::forearm_command_process(const std::string& method, const std::string& action, const std::string& parameter)
{
    OATPP_LOGD(TAG, "Forearm processor: method = %s, action = %s, parameter = %s",
               method.c_str(), action.c_str(), parameter.c_str());

    // Only changing device state was implemented.
    if (method != "PUT") return "";

    std::string command("!");

    if ("rotate_angle" == action) command += "r";
    else if ("lift_angle" == action) command += "l";
    else return "";

    command += "f" + parameter + "!";

    return command;
}


std::string WSListener::manipulator_command_process(const std::string& method, const std::string& action, const std::string& parameter)
{
    OATPP_LOGD(TAG, "Manipulator processor: method = %s, action = %s, parameter = %s",
               method.c_str(), action.c_str(), parameter.c_str());

    // Only changing device state was implemented.
    if (method != "PUT") return "";

    std::string command("!");

    if ("lift_angle" == action) command += "l";
    else if ("open_angle" == action) command += "r";
    else return "";

    command += "m" + parameter + "!";

    return command;
}

