#include "server_web.h"
#include "web_socket.h"

#include <memory>
#include <iostream>
#include <type_traits>
#include <utility>

#include <restbed>


using namespace org::openapitools::server;
using namespace boost::bimaps;


StateStringConverter StateStringConverter::ssc_;


StateStringConverter::StateStringConverter()
{
    bm_.insert({DeviceStateEnum::unknown, "unknown"});
    bm_.insert({DeviceStateEnum::enabled, "enabled"});
    bm_.insert({DeviceStateEnum::disabled, "disabled"});
    bm_.insert({DeviceStateEnum::rebooting, "rebooting"});
}


std::string StateStringConverter::from_state(const DeviceStateEnum &ds) const
{
    auto i = bm_.left.find(ds);
    return (i != bm_.left.end()) ? i->second : "unknown";
}


DeviceStateEnum StateStringConverter::to_state(const std::string &s) const
{
    auto i = bm_.right.find(s);
    return (i != bm_.right.end()) ? i->second : DeviceStateEnum::unknown;
}


template<typename H, typename... Ts>
typename std::invoke_result<H, Ts...>::type execute_handler(H handler, Ts... args)
{
    if (handler)
    {
        try
        {
            return handler(args...);
        }
        catch(const api::DeviceApiException& e)
        {
            throw e;
        }
        catch(const std::exception &e)
        {
            throw api::DeviceApiException(500, std::string("Internal Server Error: ") + e.what());
        }
        catch(...)
        {
            throw api::DeviceApiException(500, "Internal Server Error");
        }
    }
    else
    {
        throw api::DeviceApiException(501, "Not Implemented");
    }
}


std::pair<int, std::vector<std::string>> DevicesResource::handler_GET(
        int32_t const & skip, int32_t const & limit)
{
    return execute_handler(get_handler_, skip, limit);
}


int DevicesResource::handler_POST(
        std::shared_ptr<api::DeviceRegistrationInfo> const & deviceRegistrationInfo)
{
    return execute_handler(post_handler_, deviceRegistrationInfo->getId(), deviceRegistrationInfo->getName());
}


std::pair<int, std::shared_ptr<api::DeviceInfo>> DeviceInfoResource::handler_GET(
    std::string const & deviceId)
{
    auto di = std::make_shared<api::DeviceInfo>();

    auto [result, name, device_state] = execute_handler(get_handler_, deviceId);

    boost::property_tree::ptree pt;
    pt.put("state", StateStringConverter::get_instance().from_state(device_state));

    di->setName(name);
    di->setState(std::make_shared<api::DeviceState>(pt));
    return std::make_pair(result, di);
}


int DeviceInfoResource::handler_DELETE(
    std::string const & deviceId)
{
    return execute_handler<>(delete_handler_, deviceId);
}


int DeviceStateResource::handler_PUT(
    std::string const & deviceId, std::string const & body)
{
    api::DeviceState ds;
    ds.fromJsonString(body);

    return execute_handler(put_handler_, deviceId, StateStringConverter::get_instance().to_state(ds.toPropertyTree().get("state", "unknown")));
}


std::pair<int, std::shared_ptr<api::DeviceState>> DeviceStateResource::handler_GET(
        std::string const & deviceId)
{
    auto res = execute_handler(get_handler_, deviceId);
    boost::property_tree::ptree pt;
    pt.put("state", StateStringConverter::get_instance().from_state(res.second));

    return std::make_pair(res.first, std::make_shared<api::DeviceState>(pt));
}


void run_web_server(int port,
                    std::shared_ptr<DevicesResource> devices,
                    std::shared_ptr<DeviceInfoResource> device_info,
                    std::shared_ptr<DeviceStateResource> device_state)
{
    using namespace org::openapitools::server;

    auto settings = std::make_shared<restbed::Settings>();
    settings->set_port(port);

    auto service = std::make_shared<restbed::Service>();

    api::DevicesApi das(service);
    api::DeviceApi da(service);
    api::DeviceStateApi dsa(service);

    das.setDevicesApiDevicesResource(devices);
    da.setDeviceApiDeviceDeviceIdResource(device_info);
    dsa.setDeviceStateApiDeviceDeviceIdStateResource(device_state);

    das.publishDefaultResources();
    da.publishDefaultResources();
    dsa.publishDefaultResources();

    ws_create(service);

    service->start(settings);
}

