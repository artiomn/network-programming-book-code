extern "C"
{
#include <modbus.h>
}

#include <iostream>


int main()
{
    modbus_t *ctx = modbus_new_rtu("/dev/ttyS0", 9600, 'N', 8, 1);

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

    modbus_set_slave(ctx, 3);

    uint16_t reg[5];

    int num = modbus_read_registers(ctx, 10, 5, reg);
    if (num != 5)
    {
        std::cerr << "Failed to read: " << modbus_strerror(errno) << std::endl;
    }

    modbus_close(ctx);
    modbus_free(ctx);
}
