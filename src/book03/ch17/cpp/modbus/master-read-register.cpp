extern "C"
{
#include <modbus.h>
}

#include <iostream>


int main()
{
    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 1502);
    uint16_t tab_reg[32];
    if (!ctx)
    {
        std::cerr << "Failed to create the context: " << modbus_strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    if (-1 == modbus_connect(ctx))
    {
        std::cerr << "Unable to connect: " << modbus_strerror(errno) << std::endl;
        modbus_free(ctx);
        return EXIT_FAILURE;
    }

    modbus_read_registers(ctx, 0, 5, tab_reg);

    modbus_close(ctx);
    modbus_free(ctx);
}
