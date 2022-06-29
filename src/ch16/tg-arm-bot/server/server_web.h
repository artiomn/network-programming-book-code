#pragma once

#include <functional>
#include <memory>
#include <tuple>
#include <utility>

#include <boost/bimap.hpp>
#include <restbed>

#include <DevicesApi.h>
#include <DeviceApi.h>
#include <DeviceStateApi.h>


enum class DeviceStateEnum
{
    unknown,
    enabled,
    disabled,
    rebooting
};


class StateStringConverter
{
public:
    static const StateStringConverter& get_instance() { return ssc_; };

private:
    StateStringConverter();

public:
    std::string from_state(const DeviceStateEnum &ds) const;

    DeviceStateEnum to_state(const std::string &s) const;

private:
    boost::bimaps::bimap<DeviceStateEnum, std::string> bm_;
    static StateStringConverter ssc_;
};


class DevicesResource: public org::openapitools::server::api::DevicesApiDevicesResource
{
public:
    typedef std::function<std::pair<int, std::vector<std::string>>(const int32_t &skip, const int32_t& limit)> GetHandler;
    typedef std::function<int(const std::string &device_id, const std::string &name)> PostHandler;

public:
    void set_get_handler(GetHandler h) { get_handler_ = h; }
    void set_post_handler(PostHandler h) { post_handler_ = h; }

protected:
    std::pair<int, std::vector<std::string>> handler_GET(
        int32_t const & skip, int32_t const & limit) override;

    int handler_POST(
        std::shared_ptr<org::openapitools::server::api::DeviceRegistrationInfo> const & deviceRegistrationInfo) override;

private:
    GetHandler get_handler_;
    PostHandler post_handler_;
};


class DeviceInfoResource : public org::openapitools::server::api::DeviceApiDeviceDeviceIdResource
{
public:
    typedef std::function<std::tuple<int, std::string, DeviceStateEnum>(const std::string &device_id)> GetHandler;
    typedef std::function<int(const std::string &device_id)> DeleteHandler;

public:
    void set_get_handler(GetHandler h) { get_handler_ = h; }
    void set_delete_handler(DeleteHandler h) { delete_handler_ = h; }

protected:
    std::pair<int, std::shared_ptr<org::openapitools::server::api::DeviceInfo>> handler_GET(
        std::string const & deviceId) override;

    int handler_DELETE(std::string const & deviceId) override;

private:
    GetHandler get_handler_;
    DeleteHandler delete_handler_;
};


class DeviceStateResource : public org::openapitools::server::api::DeviceStateApiDeviceDeviceIdStateResource
{
public:
    typedef std::function<std::pair<int, DeviceStateEnum>(const std::string &device_id)> GetHandler;
    typedef std::function<int(const std::string &device_id, const DeviceStateEnum &state)> PutHandler;

public:
    void set_get_handler(GetHandler h) { get_handler_ = h; }
    void set_put_handler(PutHandler h) { put_handler_ = h; }

protected:
    std::pair<int, std::shared_ptr<org::openapitools::server::api::DeviceState>> handler_GET(
        std::string const & deviceId) override;

    int handler_PUT(
        std::string const & deviceId, std::string const & body) override;

private:
    GetHandler get_handler_;
    PutHandler put_handler_;
};


void run_web_server(int port,
                    std::shared_ptr<DevicesResource> devices,
                    std::shared_ptr<DeviceInfoResource> device_info,
                    std::shared_ptr<DeviceStateResource> device_state);

