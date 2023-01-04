#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <restbed>


typedef std::function<void(const std::string &device_id, const std::string& command)> WsMessageHandler;


void ws_create(std::shared_ptr<restbed::Service> service,
               const std::string &url = "/artiomn/robotic_arm_server/1.0.0/",
               long ping_timeout = 5000);

void ws_send_command(const std::string &device_id, const std::string& command);
void ws_on_message(WsMessageHandler on_message);

// TODO: Make ws_on_close and ws_on_error handlers!
