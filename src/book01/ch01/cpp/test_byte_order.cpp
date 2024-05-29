/*
 * Get byte-order for the machine.
 */

#include <cstdint>
#include <iostream>

int main()
{
    uint16_t x = 0x0001;
    // cppcheck-suppress knownConditionTrueFalse
    std::cout << (*(reinterpret_cast<uint8_t*>(&x)) ? "little" : "big") << "-endian" << std::endl;
    return EXIT_SUCCESS;
}
