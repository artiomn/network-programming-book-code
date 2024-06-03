extern "C"
{
#include <nb30.h>
}


#include <cassert>
#include <iostream>
#include <string>
#include <vector>


int main(int argc, char* argv[])
{
    LANA_ENUM l_enum;

    NCB ncb = {.ncb_command = NCBENUM, .ncb_buffer = reinterpret_cast<PUCHAR>(lenum), .ncb_length = sizeof(LANA_ENUM)};

    if (NRC_GOODRET != Netbios(&ncb))
    {
        return ncb.ncb_retcode;
    }

    std::vector<char> data_buffer(1024);

    std::string local_name = "corp.node_name.com";

    for (int i = 0; i < l_enum.length; ++i)
    {
        std::fill_n(static_cast<char*>(&ncb), sizeof(NCB), 0);
        ncb.ncb_command = NCBASTAT;
        ncb.ncb_lana_num = reinterpret_cast<int>(l_enum.lana[i]);
        ncb.ncb_buffer = static_cast<PUCHAR>(data_buffer.data());
        ncb.ncb_length = data_buffer.size();

        std::copy(local_name.c_str(), local_name.c_str() + local_name.size(), ncb.ncb_callname);

        Netbios(&ncb);

        if (NRC_GOODRET != ncb.ncb_retcode) std::cerr << "Error" << std::endl;
    }

    return EXIT_SUCCESS;
}
