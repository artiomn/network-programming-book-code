extern "C"
{
#include <modbus.h>
}

#include <array>
#include <iostream>


int main()
{
    int res = EXIT_SUCCESS;

    modbus_mapping_t *mapping = modbus_mapping_new(0, 1, 30, 2);
    if (!mapping)
    {
        std::cerr << "Failed to allocate the mapping: " << modbus_strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    mapping->tab_registers[12] = 623;

    modbus_t *ctx = nullptr;

    try
    {
        // ctx = modbus_new_rtu("/dev/ttyS0", 9600, 'N', 8, 1);
        ctx = modbus_new_tcp(nullptr, 1502);
        if (!ctx) throw std::logic_error(modbus_strerror(errno));

        // modbus_set_slave(ctx, 3);

        auto s = modbus_tcp_listen(ctx, 1);

        if (-1 == s) throw std::logic_error(modbus_strerror(errno));
        if (-1 == modbus_tcp_accept(ctx, &s)) throw std::logic_error(modbus_strerror(errno));

        std::array<uint8_t, MODBUS_RTU_MAX_ADU_LENGTH> buf;

        while (true)
        {
            int len = modbus_receive(ctx, &buf[0]);
            std::cout << "Received " << len << " bytes." << std::endl;
            if (-1 == len) break;
            len = modbus_reply(ctx, buf.data(), len, mapping);
            if (-1 == len) break;
        }

        std::cerr << "Exiting the loop: " << modbus_strerror(errno) << std::endl;
    }
    catch (const std::exception &e)
    {
        res = EXIT_FAILURE;
    }

    if (ctx)
    {
        modbus_close(ctx);
        modbus_free(ctx);
    }

    modbus_mapping_free(mapping);

    return res;
}
