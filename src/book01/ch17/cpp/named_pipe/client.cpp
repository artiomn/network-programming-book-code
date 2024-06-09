#include <windows.h>

#include <iostream>
#include <string>
#include <vector>


int main(int argc, char** argv)
{
    using tstring = std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>>;
    tstring message = TEXT("I'm message from client");
    std::vector<TCHAR> read_buffer(1024);
    const tstring pipe_name = TEXT(R"(\\.\pipe\test_pipe)");

    if (argc > 1)
    {
        message = argv[1];
    }

    // Try to open a named pipe; wait for it, if necessary.
    HANDLE h_pipe = CreateFile(pipe_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

    if (INVALID_HANDLE_VALUE == h_pipe)
    {
        std::cerr << "Pipe creating error!" << std::endl;
        return EXIT_FAILURE;
    }

    // if (!WaitNamedPipe(pipe_name.c_str(), 1000))
    //{
    //     std::cerr << "Waiting error [" << std::system_category().message(GetLastError()) << "]!" << std::endl;
    //     CloseHandle(h_pipe);
    //     return EXIT_FAILURE;
    // }

    // The pipe connected; change to message-read mode.
    DWORD mode = PIPE_READMODE_MESSAGE;
    // Don't set maximum bytes and maximum time.
    if (!SetNamedPipeHandleState(h_pipe, &mode, nullptr, nullptr))
    {
        std::cerr << "SetNamedPipeHandleState failed." << std::endl;
        return EXIT_FAILURE;
    }

    DWORD read_bytes;

    // Send a message to the pipe server and read the response.
    if (!TransactNamedPipe(
            h_pipe, const_cast<TCHAR*>(message.c_str()), message.size() * sizeof(decltype(message)::value_type),
            read_buffer.data(), read_buffer.size() * sizeof(decltype(read_buffer)::value_type), &read_bytes, nullptr) &&
        (GetLastError() != ERROR_MORE_DATA))
    {
        CloseHandle(h_pipe);
        std::cerr << "TransactNamedPipe failed." << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "Data: " << std::string(read_buffer.begin(), read_buffer.begin() + read_bytes) << std::endl;
    }

    CloseHandle(h_pipe);

    return EXIT_SUCCESS;
}
