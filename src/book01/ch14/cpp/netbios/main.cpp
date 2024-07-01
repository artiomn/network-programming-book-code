extern "C"
{
// clang-format off
// Windows header must be first.
#include <windows.h>
#include <nb30.h>
    // clang-format on
}


#include <array>
#include <cassert>
#include <iostream>
#include <string>


int main()
{
    LANA_ENUM l_enum;

    NCB ncb;
    ncb.ncb_command = NCBENUM;
    ncb.ncb_buffer = reinterpret_cast<PUCHAR>(&l_enum);
    ncb.ncb_length = sizeof(LANA_ENUM);

    if (NRC_GOODRET != Netbios(&ncb))
    {
        return ncb.ncb_retcode;
    }

    std::cout << "LANA count = " << +l_enum.length << std::endl;

    std::array<UCHAR, 1024> data_buffer;

    constexpr char local_name[] = "corp.node_name.com";

    for (int i = 0; i < l_enum.length; ++i)
    {
        std::fill_n(reinterpret_cast<char*>(&ncb), sizeof(NCB), 0);
        ncb.ncb_command = NCBASTAT;
        ncb.ncb_lana_num = l_enum.lana[i];
        ncb.ncb_buffer = data_buffer.data();
        ncb.ncb_length = data_buffer.size();

        std::copy(std::begin(local_name), std::end(local_name), ncb.ncb_callname);

        Netbios(&ncb);

        if (NRC_GOODRET != ncb.ncb_retcode) std::cerr << "Error" << std::endl;
    }

    return EXIT_SUCCESS;
}
