extern "C"
{
#include <modbus.h>
}

#include <array>
#include <iostream>


int main()
{
    modbus_t *ctx = modbus_new_tcp("127.0.0.1", 1502);
    // modbus_t *ctx = modbus_new_rtu("/dev/ttyS0", 9600, 'N', 8, 1);

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

    // modbus_set_slave(ctx, 3);

    std::array<uint16_t, 5> reg;

    int num = modbus_read_registers(ctx, 10, reg.size(), reg.data());
    if (num != 5)
    {
        std::cerr << "Failed to read: " << modbus_strerror(errno) << std::endl;
    }
    else
        std::cout << "Received " << num << " bytes." << std::endl;

    modbus_close(ctx);
    modbus_free(ctx);
}
