#include <windows.h>

#include <iostream>
#include <vector>


int main()
{
    HANDLE h_pipe;
    LPTSTR message = TEXT("Default message from client");
    std::vector<TCHAR> read_buffer(512);
    const LPTSTR pipe_name = TEXT("\\\\.\\pipe\\test_pipe");

    if (argc > 1)
    {
        message = argv[1];
    }

    // Try to open a named pipe; wait for it, if necessary.
    HANDLE h_pipe = CreateFile(pipe_name, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

    if (INVALID_HANDLE_VALUE == h_pipe)
    {
        std::cerr << "Pipe creating error!" << std::endl;
    }

    // Exit if an error other than ERROR_PIPE_BUSY occurs.
    if (GetLastError() != ERROR_PIPE_BUSY)
    {
        std::cerr << "Could not open pipe" << std::endl;
        return EXIT_FAILURE;
    }

    // All pipe instances are busy, so wait for 20 seconds.
    if (!WaitNamedPipe(pipe_name, 20000))
    {
        std::cerr << "Could not open pipe" << std::endl;
        return EXIT_FAILURE;
    }

    // The pipe connected; change to message-read mode.
    DWORD mode = PIPE_READMODE_MESSAGE;
    // Don't set maximum bytes and maximum time.
    auto f_success = SetNamedPipeHandleState(h_pipe, mode, nullptr, nullptr);
    if (!f_success)
    {
        std::cerr << "SetNamedPipeHandleState failed." << std::endl;
        return EXIT_FAILURE;
    }

    // Send a message to the pipe server and read the response.
    f_success = TransactNamedPipe(
        h_pipe, message, (lstrlen(message) + 1) * sizeof(TCHAR), chReadBuf,
        read_buffer.size() * sizeof(read_buffer::value_type), nullptr, nullptr);

    if (!f_success && (GetLastError() != ERROR_MORE_DATA))
    {
        std::cerr << "TransactNamedPipe failed." << std::endl;
        return EXIT_FAILURE;
    }

    while (true)
    {
        std::cout << "Data: " << std::string(read_buffer.begin(), read_buffer.end()) << std::endl;

        // Break if TransactNamedPipe or ReadFile is successful
        if (f_success) break;

        // Read from the pipe if there is more data in the message.
        f_success = ReadFile(
            h_pipe, read_buffer.data(), read_buffer.size() * sizeof(read_buffer::value_type), nullptr, nullptr);

        // Exit if an error other than ERROR_MORE_DATA occurs.
        if (!f_success && (GetLastError() != ERROR_MORE_DATA))
            break;
        else
            std::cout << "Data: " << std::string(read_buffer.begin(), read_buffer.end()) << std::endl;
    }

    _getch();

    CloseHandle(h_pipe);

    return EXIT_SUCCESS;
}
