#pragma once

#include <string>


class SerialPort final
{
public:
    SerialPort(const std::string &path) : port_path_(path)
    {
//        open();
    }
    ~SerialPort() { close(); };

public:
    void open();
    void close();
    int write(const std::string& s);
    std::string read();

public:
    bool opened() const { return port_descriptor_ > -1; }
    int descriptor() const { return port_descriptor_; }

private:
    void set_parameters();

private:
    static constexpr const char *TAG = "Serial_Port";

private:
    const std::string port_path_;
    int port_descriptor_ = -1;
};
