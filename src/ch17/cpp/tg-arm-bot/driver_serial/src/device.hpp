#pragma once

#include "oatpp/web/client/ApiClient.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"


#include OATPP_CODEGEN_BEGIN(DTO)

// Device Registration Info data to JSON mapper.
class DeviceRegistrationInfoDTO : public oatpp::DTO
{
    DTO_INIT(DeviceRegistrationInfoDTO, DTO)

    DTO_FIELD(String, id);
    DTO_FIELD(String, name);
};

#include OATPP_CODEGEN_END(DTO)


// API class.
class TgArmBotApiClient : public oatpp::web::client::ApiClient
{
#include OATPP_CODEGEN_BEGIN(ApiClient)

    API_CLIENT_INIT(TgArmBotApiClient)

    API_CALL("POST", "{api_endpoint}devices", register_device,
             PATH(String, api_endpoint),
             BODY_DTO(Object<DeviceRegistrationInfoDTO>, registration_info))
    API_CALL("DELETE", "{api_endpoint}/device/{uid}", unregister_device,
             PATH(String, api_endpoint),
             PATH(String, uid))
    API_CALL("PUT", "{api_endpoint}device/{uid}", set_device_state,
             PATH(String, api_endpoint),
             PATH(String, uid),
             BODY_STRING(String, device_state))

#include OATPP_CODEGEN_END(ApiClient)
};
