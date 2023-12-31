#include <modbus.h>

#include <iostream>


int main()
{
    modbus_mapping_t *mapping = modbus_mapping_new(0, 1, 30, 2);
    if (!mapping)
    {
        std::cerr << "Failed to allocate the mapping: " << modbus_strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    mapping->tab_registers[12] = 623;

    modbus_t *ctx = modbus_new_rtu("/dev/ttyS0", 9600, 'N', 8, 1);
    if (!ctx)
    {
        std::cerr << "Failed to create the context: " << modbus_strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    modbus_set_slave(ctx, 3);

    if (-1 == modbus_connect(ctx))
    {
        stderr << "Unable to connect: " << modbus_strerror(errno) << std::endl;
        modbus_free(ctx);
        return EXIT_FAILURE;
    }

    uint8_t buf[MODBUS_RTU_MAX_ADU_LENGTH];

    while (true)
    {
        int len = modbus_receive(ctx, buf);
        if (-1 == len) break;
        len = modbus_reply(ctx, buf, len, mapping);
        if (-1 == len) break;
    }

    std::cerr << "Exiting the loop: " << modbus_strerror(errno) << std::endl;

    modbus_mapping_free(mapping);
    modbus_close(ctx);
    modbus_free(ctx);
}
