/*
 * Get byte-order for the machine.
 */

#include <iostream>
#include <cstdint>

int main()
{
    uint16_t x = 0x0001;
    std::cout << (*((uint8_t*) &x) ? "little" : "big") << "-endian" << std::endl;
}
