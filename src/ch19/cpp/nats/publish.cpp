extern "C"
{
#include <nats/nats.h>
}

#include <iostream>
#include <string>


int main(int argc, char **argv)
{
    natsConnection *conn = nullptr;
    natsStatistics *stats = nullptr;
    natsOptions *opts = nullptr;
    natsStatus s = NATS_OK;
    const size_t total = 10;
    const std::string subj = "test";
    const std::string payload = "test pl";

    if (argc < 2)
    {
        std::cerr << argv[0] << " <nats address>" << std::endl;
        return EXIT_FAILURE;
    }

    if (natsOptions_Create(&opts) != NATS_OK) s = NATS_NO_MEMORY;

    const char *server_urls[1] = {argv[1]};

    if (NATS_OK == s)
        s = natsOptions_SetServers(
            opts, reinterpret_cast<const char **>(server_urls), sizeof(server_urls) / sizeof(*server_urls));

    if (NATS_OK == s) s = natsConnection_Connect(&conn, opts);

    if (NATS_OK == s) s = natsStatistics_Create(&stats);

    std::cout << "Sending " << total << " messages to subject '" << subj << "'..." << std::endl;

    for (size_t count = 0; (NATS_OK == s) && (count < total); ++count)
        s = natsConnection_Publish(conn, subj.c_str(), reinterpret_cast<const void *>(payload.c_str()), payload.size());

    if (NATS_OK == s) s = natsConnection_FlushTimeout(conn, 1000);

    if (NATS_OK == s)
    {
        std::cout << "Sent" << std::endl;
    }
    else
    {
        std::cerr << "Error: " << s << " - " << natsStatus_GetText(s) << std::endl;
        nats_PrintLastErrorStack(stderr);
    }

    natsStatistics_Destroy(stats);
    natsConnection_Destroy(conn);
    natsOptions_Destroy(opts);

    nats_Close();

    return EXIT_SUCCESS;
}
