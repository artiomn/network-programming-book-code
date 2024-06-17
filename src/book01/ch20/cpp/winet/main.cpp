extern "C"
{
#include <windows.h>
#include <wininet.h>
#include <winsock.h>
}


#include <array>
#include <iostream>


int main()
{
    const auto internet_handle = InternetOpen(TEXT("Mozilla/5.12345"), INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!internet_handle)
    {
        std::cerr << "InternetOpen failed with code " << GetLastError() << std::endl;
        return EXIT_FAILURE;
    }

    const auto url_handle =
        InternetOpenUrl(internet_handle, TEXT("http://artiomsoft.ru"), 0, 0, INTERNET_FLAG_RAW_DATA, 0);

    if (!url_handle)
    {
        std::cerr << "InternetOpenUrl failed with code " << GetLastError() << std::endl;
        InternetCloseHandle(internet_handle);
        return EXIT_FAILURE;
    }

    std::array<TCHAR, 1024> buf;
    DWORD read_bytes = 0;

    while (InternetReadFile(url_handle, buf.data(), static_cast<DWORD>(buf.size()), &read_bytes))
    {
        if (!read_bytes) break;
        std::ostream_iterator<TCHAR> out_it(std::cout, "");
        std::copy(buf.begin(), buf.begin() + read_bytes, out_it);
    }

    InternetCloseHandle(url_handle);
    InternetCloseHandle(internet_handle);
    return EXIT_SUCCESS;
}
