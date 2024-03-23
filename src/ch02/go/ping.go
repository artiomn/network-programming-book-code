package main

import "fmt"
import "net"
import "os"
import "time"
import "github.com/tatsushid/go-fastping"



func main() {
    p := fastping.NewPinger()
    ra, err := net.ResolveIPAddr("ip4:icmp", os.Args[1])
    if err != nil {
        fmt.Println(err)
        os.Exit(1)
    }
    p.AddIPAddr(ra)
    p.OnRecv = func(addr *net.IPAddr, rtt time.Duration) {
        fmt.Printf("IP Addr: %s receive, RTT: %v\n", addr.String(), rtt)
    }
    p.OnIdle = func() {
        fmt.Println("finish")
    }
    err = p.Run()
    if err != nil {
        fmt.Println(err)
    }
}
