extern "C"
{
#include <windows.h>
}

#include <array>
#include <iostream>


int main()
{
    const LPCTSTR slot_name = TEXT("\\\\.\\mailslot\\test_mailslot");
    HANDLE h_slot = CreateMailslot(slot_name, 0, MAILSLOT_WAIT_FOREVER, nullptr);

    if (INVALID_HANDLE_VALUE == h_slot)
    {
        std::cerr << "CreateMailslot failed" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Mailslot created successfully." << std::endl;
    std::array<char, 2048> buffer;
    DWORD bytes_read = 0;

    while (ReadFile(h_slot, &buffer[0], buffer.size(), &bytes_read, nullptr))
    {
        std::cout << "Received: " << std::string(buffer.begin(), buffer.begin() + bytes_read) << std::endl;
    }

    CloseHandle(h_slot);
    return EXIT_SUCCESS;
}
