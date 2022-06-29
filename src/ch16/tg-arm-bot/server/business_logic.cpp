#include "business_logic.h"

#include <cinttypes>
#include <sstream>


void ServerBusinessLogic::run()
{
    create_web_server();

    create_and_run_tg_bot(bot_token_,
        [this](TgBot::Bot &bot, const int64_t chat_id, std::vector<std::string> &&command,
           const std::optional<int> &parameter) -> bool
    {
        if (!command.size()) return false;

        std::cout << "Cmd: ";
        for (auto cmd : command)
            std::cout
                << cmd << " ";
        std::cout << std::endl;

        if ("status" == command[0]) return cmd_status_process(bot, chat_id);
        if ("reboot" == command[0]) return cmd_reboot_process(bot, chat_id);
        if ("shutdown" == command[0]) return cmd_shutdown_process(bot, chat_id);

        if (std::nullopt == parameter)
        {
            std::cerr
                << "Unset parameter!"
                << std::endl;
            return false;
        }
        else
        {
            std::cout << "Parameter = " << *parameter << std::endl;
        }

        if ("shoulder" == command[0]) return cmd_shoulder_process(bot, chat_id, std::move(command), *parameter);
        if ("forearm" == command[0]) return cmd_forearm_process(bot, chat_id, std::move(command), *parameter);
        if ("manipulator" == command[0]) return cmd_manipulator_process(bot, chat_id, std::move(command), *parameter);

        std::cerr
            << "Unknown command prefix: "
            << command[0]
            << std::endl;

        return true;
    });
}


void ServerBusinessLogic::create_web_server()
{
    auto devices = std::make_shared<DevicesResource>();
    auto device_info = std::make_shared<DeviceInfoResource>();
    auto device_state = std::make_shared<DeviceStateResource>();

    devices->set_get_handler([this](const int32_t &skip, const int32_t& limit)
    {
        return devices_get_handler(skip, limit);
    });

    devices->set_post_handler([this](const std::string &device_id, const std::string& name)
    {
        return devices_post_handler(device_id, name);
    });

    device_info->set_get_handler([this](const std::string& device_id)
    {
        return device_get_handler(device_id);
    });

    device_info->set_delete_handler([this](const std::string& device_id)
    {
        return device_delete_handler(device_id);
    });

    device_state->set_get_handler([this](const std::string& device_id)
    {
        return device_state_get_handler(device_id);
    });

    device_state->set_put_handler([this](const std::string& device_id, const DeviceStateEnum& state)
    {
        return device_state_put_handler(device_id, state);
    });

    std::thread server_thread(run_web_server, web_server_port_, devices, device_info, device_state);
    server_thread.detach();

}


std::pair<int, std::vector<std::string>> ServerBusinessLogic::devices_get_handler(const int32_t &skip, const int32_t& limit)
{
    std::vector<std::string> devices;
    devices.reserve(devices_map_.size());

    for (const auto& [key, value] : devices_map_) devices.push_back(key);

    return std::make_pair(200, devices);
}


int ServerBusinessLogic::devices_post_handler(const std::string &device_id, const std::string &name)
{
    if (device_id.empty() || name.empty())
    {
        std::stringstream ss;

        ss << "Incorrect device parameters!";
        std::cerr << ss.str() << std::endl;

        throw org::openapitools::server::api::DeviceApiException(400, ss.str());

        // Possibly variant (but it's not good, because message will not returned to the client):
        // return 400;
    }

    if (auto it{ devices_map_.find(device_id) }; it != devices_map_.end())
    {
        std::stringstream ss;

        ss
            << "Device \"" << device_id << "\" "
            << "was already registered!"
            << std::endl;
        std::cerr << ss.str() << std::endl;

        throw org::openapitools::server::api::DeviceApiException(403, ss.str());
    }

    std::cout
        << "Device \"" << device_id << "\" "
        << "was registered with name \"" << name << "\""
        << std::endl;

    devices_map_.emplace(std::make_pair(device_id, Device(name, DeviceStateEnum::unknown)));

    return 200;
}


std::tuple<int, std::string, DeviceStateEnum> ServerBusinessLogic::device_get_handler(const std::string &device_id)
{
    auto it = check_device_existing(device_id);

    std::cout
        << "Get device: "
        << "\"" << it->second.name_ << "\": "
        << StateStringConverter::get_instance().from_state(it->second.state_)
        << std::endl;

    return std::make_tuple(200, it->second.name_, it->second.state_);
}


int ServerBusinessLogic::device_delete_handler(const std::string &device_id)
{
    check_device_existing(device_id);

    std::cout
        << "Erase device: " << device_id
        << std::endl;

    devices_map_.erase(device_id);

    return 200;
}


std::pair<int, DeviceStateEnum> ServerBusinessLogic::device_state_get_handler(const std::string &device_id)
{
    check_device_existing(device_id);
    auto it = check_device_existing(device_id);

    std::cout
        << "Device \"" << device_id << "\" state: "
        << StateStringConverter::get_instance().from_state(it->second.state_)
        << std::endl;

    return std::make_pair(200, it->second.state_);
}


int ServerBusinessLogic::device_state_put_handler(const std::string &device_id, const DeviceStateEnum &state)
{
    auto it = check_device_existing(device_id);

    std::cout
        << "Device \"" << device_id << "\" state: "
        << StateStringConverter::get_instance().from_state(it->second.state_)
        << " -> "
        << StateStringConverter::get_instance().from_state(state)
        << std::endl;

    it->second.state_ = state;

    return 200;
}


bool ServerBusinessLogic::cmd_status_process(TgBot::Bot& bot, const int64_t chat_id)
{
    std::stringstream ss;

    for (const auto &[k, v] : devices_map_)
    {
        ss.str("");
        ss
            << "UID:   " << k << "\n"
            << "Name:  " << v.name_ << "\n"
            << "State: " << StateStringConverter::get_instance().from_state(v.state_);
        bot.getApi().sendMessage(chat_id, ss.str());
    }

    return true;
}


bool ServerBusinessLogic::cmd_reboot_process(TgBot::Bot& bot, const int64_t chat_id)
{
    for (const auto& [key, _] : devices_map_)
    {
        set_device_state(key, DeviceStateEnum::rebooting);
    }
    return true;
}


bool ServerBusinessLogic::cmd_shutdown_process(TgBot::Bot& bot, const int64_t chat_id)
{
    for (const auto& [key, _] : devices_map_)
    {
        set_device_state(key, DeviceStateEnum::disabled);
    }
    return true;
}


bool ServerBusinessLogic::cmd_shoulder_process(TgBot::Bot& bot, const int64_t chat_id, std::vector<std::string>&& command, int parameter)
{
    for (const auto& [key, _] : devices_map_)
    {
        set_device_knob_position(key, std::move(command), parameter);
    }
    return true;
}


bool ServerBusinessLogic::cmd_forearm_process(TgBot::Bot& bot, const int64_t chat_id, std::vector<std::string>&& command, int parameter)
{
    for (const auto& [key, _] : devices_map_)
    {
        set_device_knob_position(key, std::move(command), -parameter);
    }
    return true;
}


bool ServerBusinessLogic::cmd_manipulator_process(TgBot::Bot& bot, const int64_t chat_id, std::vector<std::string>&& command, int parameter)
{
    for (const auto& [key, _] : devices_map_)
    {
        std::stringstream ss;
        ss
            << "PUT /device/" << command[0] << "/";

        if ("open" == command[1] || "close" == command[1])
        {
            ss << "open_angle" << http_suffix
               << std::to_string(("open" == command[1]) ? -parameter : parameter);
        }
        else
        {
            ss
                << "lift_angle" << http_suffix
                << std::to_string(("up" == command[1]) ? -parameter : parameter);
        }

        ws_send_command(key, ss.str());
    }
    return true;
}


void ServerBusinessLogic::set_device_state(const std::string &device_id, const DeviceStateEnum state)
{
    std::stringstream ss;
    ss
        << "PUT /device/state"
        << http_suffix
        << StateStringConverter::get_instance().from_state(state);
    ws_send_command(device_id, ss.str());
}


void ServerBusinessLogic::set_device_knob_position(const std::string &device_id, std::vector<std::string>&& command, int parameter)
{
    std::stringstream ss;
    ss
        << "PUT /device/" << command[0] << "/";

    if ("rotate" == command[1])
    {
        ss
            << "rotate_angle"
            << http_suffix
            << std::to_string(parameter);
    }
    else
    {
        ss
            << "lift_angle" << http_suffix
            << std::to_string(("up" == command[1]) ? -parameter : parameter);
    }

    ws_send_command(device_id, ss.str());
}


ServerBusinessLogic::MapIteratorType ServerBusinessLogic::check_device_existing(const std::string &device_id)
{
    if (device_id.empty())
    {
        std::stringstream ss;

        ss << "Empty device id!";
        std::cerr << ss.str() << std::endl;

        throw org::openapitools::server::api::DeviceApiException(400, ss.str());
    }

    auto it{ devices_map_.find(device_id) };

    if (devices_map_.end() == it)
    {
        std::stringstream ss;

        ss
            << "Device \"" << device_id << "\" "
            << "was not registered!";
        std::cerr << ss.str() << std::endl;
        throw org::openapitools::server::api::DeviceApiException(404, ss.str());
    }

    return it;
}

