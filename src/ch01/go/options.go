package main

import (
    "net"
    "syscall"
)


func main() {
    dialer := &net.Dialer {
        Control: func(network, address string, conn syscall.RawConn) error {
        var operr error
            if err := conn.Control(func(socket_descriptor uintptr) {
                // Вызов сокетного API.
                operr = syscall.SetsockoptInt(int(socket_descriptor),
                                              syscall.SOL_SOCKET,
                                              syscall.TCP_QUICKACK, 1)
            }); err != nil {
                return err
            }
            return operr
        },
    };
    dialer.Dial("tcp", "google.com:443");
}
