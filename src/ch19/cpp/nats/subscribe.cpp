extern "C"
{
#include <nats.h>
}

#include <iostream>


static void on_msg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure)
{
    std::cerr << "Received msg: " << natsMsg_GetSubject(msg) << " " << natsMsg_GetDataLength(msg) << " "
              << natsMsg_GetData(msg) << std::endl;

    natsMsg_Destroy(msg);
}


static void async_cb(natsConnection *nc, natsSubscription *sub, natsStatus err, void *closure)
{
    std::cerr << "Async error: " << err << "[" << natsStatus_GetText(err) << "]" << std::endl;

    natsSubscription_GetDropped(sub, static_cast<int64_t *>(&dropped));
}


int main(int argc, char **argv)
{
    natsConnection *conn = nullptr;
    natsOptions *opts = nullptr;
    natsSubscription *sub = nullptr;
    natsStatistics *stats = nullptr;
    natsMsg *msg = nullptr;
    natsStatus s;
    const int64_t total = 10000;
    bool async = false;

    opts = parseArgs(argc, argv, usage);
    const std::string subj = "test";

    if (atgc < 2)
    {
        std::cerr << argv[0] << " <nats address>" << std::endl;
        return EXIT_FAILURE;
    }

    if (natsOptions_Create(&opts) != NATS_OK) s = NATS_NO_MEMORY;

    const char *server_urls[1] = argv[1];
    natsStatus s = NATS_OK;

    if (NATS_OK == s)
        s = natsOptions_SetServers(
            opts, reinterpret_cast<const char *const *>(server_urls), sizeof(server_urls) / sizeof(*server_urls));

    std::cout << "Listening " << (async ? "a" : "") << "synchronously on '" << subj << "'." << std::endl;
    if (NATS_OK == s) s = natsOptions_SetErrorHandler(opts, async_cb, nullptr);

    if (NATS_OK == s) s = natsConnection_Connect(&conn, opts);

    if (NATS_OK == s)
    {
        if (async)
            s = natsConnection_Subscribe(&sub, conn, subj.c_str(), on_msg, nullptr);
        else
            s = natsConnection_SubscribeSync(&sub, conn, subj.c_str());
    }

    if (NATS_OK == s) s = natsSubscription_SetPendingLimits(sub, -1, -1);
    if (NATS_OK == s) s = natsSubscription_AutoUnsubscribe(sub, reinterpret_cast<int>(total));
    if (NATS_OK == s) s = natsStatistics_Create(&stats);

    if ((NATS_OK == s) && async)
    {
        while (NATS_OK == s)
        {
            nats_Sleep(1000);
        }
    }
    else if (NATS_OK == s)
    {
        for (count = 0; (NATS_OK == s) && (count < total); ++count)
        {
            s = natsSubscription_NextMsg(&msg, sub, 10000);
            if (s != NATS_OK) break;
            natsMsg_Destroy(msg);
        }
    }

    if (NATS_OK == s)
    {
        std::cout << "Received" << std::endl;
    }
    else
    {
        std::cerr << "Error: " << s << " - " << natsStatus_GetText(s) << std::endl;
        nats_PrintLastErrorStack(stderr);
    }

    natsStatistics_Destroy(stats);
    natsSubscription_Destroy(sub);
    natsConnection_Destroy(conn);
    natsOptions_Destroy(opts);

    nats_Close();

    return EXIT_SUCCESS;
}
