/*
 * AUTHOR: Matthieu Berger.
 * Example source: https://github.com/mamalovesyou/go-tcp-server.
 */

package main

import (
    "bufio"
    "bytes"
    "crypto/sha1"
    "encoding/hex"
    "fmt"
    "io"
    "log"
    "net"
)

const (
    EXIT_COMMAND = "exit"
)

// Read message from a net.Conn
func Read(conn net.Conn) (string, error) {
    reader := bufio.NewReader(conn)
    var buffer bytes.Buffer
    for {
        ba, isPrefix, err := reader.ReadLine()
        if err != nil {
            // if the error is an End Of File this is still good
            if err == io.EOF {
                break
            }
            return "", err
        }
        buffer.Write(ba)
        if !isPrefix {
            break
        }
    }
    return buffer.String(), nil
}

// Write message to a net.Conn
// Return the number of bytes returned
func Write(conn net.Conn, encoded string) (int, error) {
    writer := bufio.NewWriter(conn)
    number, err := writer.WriteString(encoded)
    if err == nil {
        err = writer.Flush()
    }
    return number, err
}

// SHA1 hashes using sha1 algorithm
func SHA1(text string) string {
    algorithm := sha1.New()
    algorithm.Write([]byte(text))
    return hex.EncodeToString(algorithm.Sum(nil))
}

func handle(conn net.Conn) {
    defer conn.Close()
    log.Printf("Now listen: %s \n", conn.RemoteAddr().String())
    content, err := Read(conn)
    if err != nil {
        log.Printf("Listener: Read error: %s", err)
    }
    if content == EXIT_COMMAND {
        log.Println("Listener: Exit!")
        return
    }
    log.Printf("Listener: Received content: %s\n", content)
    response := fmt.Sprintf("Encoded: %s \n", SHA1(content))
    log.Printf("Listener: Response: %s\n", response)
    num, err := Write(conn, response)
    if err != nil {
        log.Printf("Listener: Write Error: %s\n", err)
    }
    log.Printf("Listener: Wrote %d byte(s) to %s \n", num, conn.RemoteAddr().String())
}


func main() {
    listener, err := net.Listen("tcp", ":8080")
    if err != nil {
        // Use fatal to exit if the listener fails to start
        log.Fatal(err)
    }
    defer listener.Close()

    for {
        conn, err := listener.Accept()
        if err != nil {
            // Print the error using a log.Fatal would exit the server
            log.Println(err)
        }
        // Using a go routine to handle the connection
        go handle(conn)
    }
}
