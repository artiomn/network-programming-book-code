package main

import (
    "fmt"
    "syscall"
)


func main() {
    if s, err := syscall.Socket(syscall.AF_INET, syscall.SOCK_STREAM,
                                syscall.IPPROTO_TCP); err != nil {
        fmt.Printf("Error message: %s\n", err)
    } else {
        fmt.Printf("Descriptor: %d\n", s)
    }
}
