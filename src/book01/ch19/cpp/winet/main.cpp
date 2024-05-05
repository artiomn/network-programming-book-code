extern "C"
{
#include <winsock.h>
#include <wininet.h>
#include <windows.h>
}


#include <iostream>
#include <vector>


int main()
{
    auto internet_handle = InternetOpen(TEXT("Mozilla/5.12345"),
        INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);


    if (internet_handle)
    {
        auto url_handle = InternetOpenUrl(internet_handle,
            TEXT("http://artiomsoft.ru"), 0, 0, INTERNET_FLAG_RAW_DATA, 0);

        if (url_handle)
        {
            std::vector<TCHAR> buf(1024);
            DWORD read_bytes;

            while (InternetReadFile(url_handle, buf.data(),
                                    buf.size(), &read_bytes))
            {
                if (!read_bytes) break;
                std::ostream_iterator<TCHAR> out_it(std::cout, "");
                std::copy(buf.begin(), buf.begin() + read_bytes, out_it);
            }

            InternetCloseHandle(url_handle);
        }

        InternetCloseHandle(internet_handle);
    }
}