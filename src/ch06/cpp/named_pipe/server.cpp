#include <windows.h>

#include <iostream>
#include <vector>


int main(int argc, const char **argv)
{
    HANDLE h_pipe;
    LPTSTR message = TEXT("Default message from client");
    std::vector<TCHAR> read_buffer(512);

    const LPTSTR pipename = TEXT("\\\\.\\pipe\\mynamedpipe");

    if (argc > 1)
    {
        message = argv[1];
    }

    // Try to open a named pipe; wait for it, if necessary.
    while (true)
    {
        h_pipe = CreateFile(pipename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

        if (h_pipe != INVALID_HANDLE_VALUE) break;

        // Exit if an error other than ERROR_PIPE_BUSY occurs.
        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            std::cerr << "Could not open pipe." << std::endl;
            return EXIT_FAILURE;
        }

        // All pipe instances are busy, so wait for 20 seconds.
        if (!WaitNamedPipe(pipename, 20000))
        {
            std::cerr << "Could not open pipe" << std::endl;
            return EXIT_FAILURE;
        }
    }

    // The pipe connected; change to message-read mode.
    DWORD mode = PIPE_READMODE_MESSAGE;
    auto f_success = SetNamedPipeHandleState(h_pipe, &mode, nullptr, nullptr);

    if (!f_success)
    {
        std::cerr << "SetNamedPipeHandleState() failed." << std::endl;
        return EXIT_FAILURE;
    }

    // Send a message to the pipe server and read the response.
    f_success = TransactNamedPipe(
        h_pipe, message, (lstrlen(message) + 1) * sizeof(TCHAR), read_buffer.data(),
        read_buffer.size() * sizeof(read_buffer::value_type), nullptr, nullptr);

    if (!f_success && (GetLastError() != ERROR_MORE_DATA))
    {
        std::cerr << "TransactNamedPipe failed." << std::endl;
        return 0;
    }

    while (true)
    {
        DWORD read_bytes;
        std::cout << "Data: " << std::string(read_buffer.begin(), read_buffer.end()) << std::endl;

        // Break if TransactNamedPipe or ReadFile is successful
        if (f_success) break;

        // Read from the pipe if there is more data in the message.
        f_success = ReadFile(
            h_pipe, read_buffer.data(), read_buffer.size() * sizeof(read_buffer::value_type), &read_bytes, nullptr);

        // Exit if an error other than ERROR_MORE_DATA occurs.
        if (!f_success && (GetLastError() != ERROR_MORE_DATA))
            break;
        else
            std::cout << "Data: " << std::string(read_buffer.begin(), read_buffer.end()) << std::endl;
    }

    _getch();

    CloseHandle(hPipe);

    return EXIT_SUCCESS;
}
