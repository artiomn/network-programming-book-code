extern "C"
{
#include <windows.h>
}

#include <iostream>
#include <vector>


int main()
{
    const LPCTSTR SlotName = TEXT("\\\\.\\mailslot\\test_mailslot");
    HANDLE h_slot = CreateMailslot(lpszSlotName, 0, MAILSLOT_WAIT_FOREVER, nullptr);

    if (INVALID_HANDLE_VALUE == h_slot)
    {
        std::cerr << "CreateMailslot failed" << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "Mailslot created successfully.";
        std::vector<char> buffer(2048);
        DWORD bytes_read;

        while (ReadFile(h_slot, &buffer[0], buffer.size(), &bytes_read, nullptr))
        {
            std::cout << "Received: " << std::string(buffer.begin(), buffer.begin() + bytes_read) << std::endl;
        }
        CloseHandle(h_slot);
    }

    return EXIT_SUCCESS;
}
