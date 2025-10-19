extern "C"
{
#include <windows.h>
}

#include <iostream>
#include <string>


int main()
{
    const LPCTSTR slot_name = TEXT(R"(\\.\mailslot\test_mailslot)");
    HANDLE h_slot =
        CreateFile(slot_name, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (INVALID_HANDLE_VALUE == h_slot)
    {
        std::cerr << "CreateFile failed" << std::endl;
        return EXIT_FAILURE;
    }

    const std::string msg = "Test message...";

    DWORD bytes_written = 0;

    if (!WriteFile(h_slot, msg.c_str(), msg.size(), &bytes_written, nullptr))
    {
        std::cerr << "Failed " << GetLastError() << std::endl;
        CloseHandle(h_slot);
        return EXIT_FAILURE;
    }

    std::cout << "Ok" << std::endl;
    CloseHandle(h_slot);
    return EXIT_SUCCESS;
}
