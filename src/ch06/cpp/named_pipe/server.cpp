#include <windows.h>

#include <iostream>
#include <string>
#include <vector>


int main(int argc, char **argv)
{
    HANDLE h_pipe;
    using tstring = std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR>>;
    tstring message = TEXT("I'm server: ");

    std::vector<TCHAR> read_buffer(512);

    const tstring pipe_name = TEXT(R"(\\.\pipe\test_pipe)");

    // Try to open a named pipe; wait for it, if necessary.
    while (true)
    {
        h_pipe = CreateNamedPipe(pipe_name.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES,
                                 message.size() * sizeof(TCHAR), read_buffer.size() * sizeof(TCHAR), NMPWAIT_USE_DEFAULT_WAIT, nullptr);

        if (h_pipe != INVALID_HANDLE_VALUE) break;

        // Exit if an error other than ERROR_PIPE_BUSY occurs.
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            std::cerr << "Could not open pipe." << std::endl;
            return EXIT_FAILURE;
        }
    }

    while (true)
    {
        std::cout << "Waiting for the connections..." << std::endl;

        if (!ConnectNamedPipe(h_pipe, nullptr))
        {
            std::cerr << "Could not connect pipe" << std::endl;
            CloseHandle(h_pipe);
            return EXIT_FAILURE;
        }

        std::cout << "Pipe connected, reading data..." << std::endl;

        // The pipe connected; change to message-read mode.
        DWORD mode = PIPE_READMODE_MESSAGE;
        if (!SetNamedPipeHandleState(h_pipe, &mode, nullptr, nullptr))
        {
            std::cerr << "SetNamedPipeHandleState() failed." << std::endl;
            CloseHandle(h_pipe);
            return EXIT_FAILURE;
        }

        // Read from the pipe if there is more data in the message.
        DWORD read_bytes;

        // Exit if an error other than ERROR_MORE_DATA occurs.
        if (!ReadFile(h_pipe, read_buffer.data(), read_buffer.size() * sizeof(decltype(read_buffer)::value_type), &read_bytes, nullptr) && (GetLastError() != ERROR_MORE_DATA))
        {
            DisconnectNamedPipe(h_pipe);
            CloseHandle(h_pipe);
            return EXIT_FAILURE;
        }
        else
        {
            std::cout << "Data read: " << std::string(read_buffer.begin(), read_buffer.begin() + read_bytes) << std::endl;
        }

        auto new_msg = message + tstring(read_buffer.begin(), read_buffer.begin() + read_bytes);

        std::cout << "Sending data..." << std::endl;

        DWORD written_bytes;
        auto result = WriteFile(
            h_pipe,
            new_msg.c_str(),
            new_msg.size() * sizeof(TCHAR),
            &written_bytes,
            nullptr
        );

        if (result)
        {
            std::cout << "Bytes sent: " << written_bytes << std::endl;
        }
        else
        {
            std::cout << "Failed to send data." << std::endl;
            CloseHandle(h_pipe);
            return EXIT_FAILURE;
        }

        if (!FlushFileBuffers(h_pipe))
        {
            std::cerr << "FlushFileBuffers() failed." << std::endl;
            CloseHandle(h_pipe);
            return EXIT_FAILURE;
        }

        if (!DisconnectNamedPipe(h_pipe))
        {
            std::cerr << "Could not connect pipe." << std::endl;
            CloseHandle(h_pipe);
            return EXIT_FAILURE;
        }
    }

    CloseHandle(h_pipe);

    return EXIT_SUCCESS;
}
