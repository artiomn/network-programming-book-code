#include "serial_port.hpp"

 #include <oatpp/core/base/Environment.hpp>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <thread>

extern "C"
{
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
}


using namespace std::chrono;


void SerialPort::open()
{
    OATPP_LOGD(TAG, "Opening port...");
    // Open the serial port.
    port_descriptor_ = ::open(port_path_.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
    //std::this_thread::sleep_for(123ms);

    if (port_descriptor_ < 0)
    {
        throw std::runtime_error(std::string("open: ") + strerror(errno));
    }

    set_parameters();
}


void SerialPort::close()
{
    if (!opened()) return;

    OATPP_LOGD(TAG, "Closing port...");
    ::close(port_descriptor_);
    port_descriptor_ = -1;
}


void SerialPort::set_parameters()
{
    OATPP_LOGD(TAG, "Setting parameters...");
    // Create new termios struct, we call it 'tty' for convention
    struct termios tty;

    // Read in existing settings, and handle any error
    if (tcgetattr(port_descriptor_, &tty) != 0)
    {
        throw std::runtime_error(std::string("tcgetattr: ") + strerror(errno));
    }

    // See: https://playground.arduino.cc/Interfacing/LinuxTTY/

    // Clear parity bit, disabling parity.
    tty.c_cflag &= ~PARENB;
    // Clear odd bit.
    tty.c_cflag &= ~PARODD;
    // 8 bits per byte.
    tty.c_cflag |= CS8;

    // -HUPCL. Flow control disable to prevent Arduino rebooting.
    tty.c_cflag &= ~HUPCL;

    // Clear stop field, only one stop bit used in communication.
    tty.c_cflag &= ~CSTOPB;

    // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_cflag |= CREAD | CLOCAL;

    // Disable RTS/CTS hardware flow control.
    tty.c_cflag &= ~CRTSCTS;

    // Turn off s/w flow ctrl.
    tty.c_iflag &= ~(IUCLC | IXANY | IMAXBEL);

    tty.c_iflag &= ~IUTF8;

    // Prevent special interpretation of output bytes (e.g. newline chars).
    tty.c_oflag &= ~OPOST;
    tty.c_oflag &= ~OLCUC;
    tty.c_oflag &= ~OCRNL;
    tty.c_oflag &= ~ONLCR;
    tty.c_oflag &= ~ONOCR;
    tty.c_oflag &= ~ONLRET;
    tty.c_oflag &= ~OFILL;
    tty.c_oflag &= ~OFDEL;

    tty.c_oflag |= (NLDLY | NL0);
    tty.c_oflag |= (CRDLY | CR0);
    tty.c_oflag |= (TABDLY | TAB0);
    tty.c_oflag |= (BSDLY | BS0);
    tty.c_oflag |= (VTDLY | VT0);
    tty.c_oflag |= (FFDLY | FF0);

    // Disable interpretation of INTR, QUIT and SUSP.
    tty.c_lflag &= ~ISIG;

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~IEXTEN;

    // Disable echo.
    tty.c_lflag &= ~ECHO;
    // Disable erasure.
    tty.c_lflag &= ~ECHOE;

    tty.c_lflag &= ~ECHOK;
    // Disable new-line echo.
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ECHOKE;

    tty.c_lflag &= ~ECHOCTL;
    tty.c_lflag &= ~ECHOPRT;
    tty.c_lflag &= ~NOFLSH;
    tty.c_lflag &= ~XCASE;
    tty.c_lflag &= ~TOSTOP;

/*

    // Prevent conversion of newline to carriage return/line feed.
    tty.c_oflag &= ~ONLCR;
    // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX).
    // tty.c_oflag &= ~OXTABS;
    // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX).
    // tty.c_oflag &= ~ONOEOT;

    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 0;
*/
    // Set in/out baud rate.
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error.
    if (tcsetattr(port_descriptor_, TCSANOW, &tty) != 0)
    {
        throw std::runtime_error(std::string("tcsetattr: ") + strerror(errno));
    }

    int flags = -1;
    if (ioctl(port_descriptor_, TIOCMGET, &flags) != 0)
    {
        throw std::runtime_error(std::string("ioctl get: ") + strerror(errno));
    }
    flags &= ~(TIOCM_CTS | TIOCM_RTS);
    if (ioctl(port_descriptor_, TIOCMSET, &flags) != 0)
    {
        throw std::runtime_error(std::string("ioctl set: ") + strerror(errno));
    }
    OATPP_LOGD(TAG, "Setting parameters finished...");
}


int SerialPort::write(const std::string &s)
{
    OATPP_LOGD(TAG, "Writing data...");
    ssize_t bytes_count = 0;
    size_t req_pos = 0;
    auto const req_buffer = &(s.c_str()[0]);
    auto const req_length = s.length();

    open();

    while (true)
    {
        if ((bytes_count = ::write(port_descriptor_, req_buffer + req_pos, req_length - req_pos)) < 0)
        {
//            if (EINTR == errno) continue;
            throw std::runtime_error(std::string("write: ") + strerror(errno));
        }
        else
        {
            if (!bytes_count) break;

            req_pos += bytes_count;

            if (req_pos >= req_length)
            {
                break;
            }
        }
    }

    OATPP_LOGD(TAG, "Writing finished...");
    close();

    return true;
}


std::string SerialPort::read()
{
    std::string read_buf;
    read_buf.resize(5);

    OATPP_LOGD(TAG, "Reading data...");

    open();

    // Read bytes. The behaviour of read() (e.g. does it block?,
    // how long does it block for?) depends on the configuration
    // settings above, specifically VMIN and VTIME
    int num_bytes = ::read(port_descriptor_, &read_buf[0], read_buf.length());

    // n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
    if (num_bytes < 0)
    {
        throw std::runtime_error(std::string("read: ") + strerror(errno));
    }

    OATPP_LOGD(TAG, "Reading finished...");
    close();

    return read_buf;
}

