#pragma once

#include <map>
#include <tuple>
#include <utility>

#include "server_tg.h"
#include "server_web.h"
#include "web_socket.h"


class ServerBusinessLogic
{
public:
    struct Device
    {
        Device(const std::string &name, DeviceStateEnum state) : name_(name), state_(state) {}
        Device(std::string &&name, DeviceStateEnum state) : name_(name), state_(state) {}

        std::string name_;
        DeviceStateEnum state_;
    };

public:
    ServerBusinessLogic(const std::string& bot_token, unsigned short port) :
        bot_token_(bot_token), web_server_port_(port)
    {
    }

public:
    void run();

private:
    void create_web_server();

private:
    std::pair<int, std::vector<std::string>> devices_get_handler(const int32_t &skip, const int32_t& limit);
    int devices_post_handler(const std::string &device_id, const std::string &name);
    std::tuple<int, std::string, DeviceStateEnum> device_get_handler(const std::string &device_id);
    int device_delete_handler(const std::string &device_id);
    std::pair<int, DeviceStateEnum> device_state_get_handler(const std::string &device_id);
    int device_state_put_handler(const std::string &device_id, const DeviceStateEnum &state);

private:
    bool cmd_status_process(TgBot::Bot&, const int64_t);
    bool cmd_reboot_process(TgBot::Bot&, const int64_t);
    bool cmd_shutdown_process(TgBot::Bot&, const int64_t);
    bool cmd_shoulder_process(TgBot::Bot&, const int64_t, std::vector<std::string>&&, int parameter);
    bool cmd_forearm_process(TgBot::Bot&, const int64_t, std::vector<std::string>&&, int parameter);
    bool cmd_manipulator_process(TgBot::Bot&, const int64_t, std::vector<std::string>&&, int parameter);

private:
    void set_device_state(const std::string &device_id, const DeviceStateEnum state);
    void set_device_knob_position(const std::string &device_id, std::vector<std::string>&& command, int parameter);

private:
    const std::string bot_token_;
    const unsigned short web_server_port_;
    std::map<std::string, Device> devices_map_;
	const std::string http_suffix = " HTTP/1.1\nContent-Type: application/json\n\n";

private:
    typedef decltype(devices_map_)::iterator MapIteratorType;
    MapIteratorType check_device_existing(const std::string &device_id);

};
