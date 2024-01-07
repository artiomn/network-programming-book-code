#include <mosquitto.h>

#include <iostream>


int on_message(mosquitto *mosq, void *userdata, const mosquitto_message *msg)
{
    std::cout << "Topic: " << msg->topic << "\n"
              << "Payload: " << reinterpret_cast<const char *>(msg->payload) << "\n"
              << "Length: " << msg->payloadlen << std::endl;
    return 0;
}


int main(int argc, const char *const argv[])
{
    int rc;

    mosquitto_lib_init();

    rc = mosquitto_subscribe_callback(
        on_message, nullptr, "Example", 0, "localhost", 1883, nullptr, 60, true, "guest", "guest", nullptr,
        // on_message, nullptr, "irc/#", 0, "test.mosquitto.org", 1883, nullptr, 60, true, nullptr, nullptr, nullptr,
        nullptr);

    if (rc)
    {
        std::cerr << "Error: " << mosquitto_strerror(rc) << std::endl;
    }

    mosquitto_lib_cleanup();

    return rc;
}
