use std::fs;
use std::io::prelude::*;
use std::net::TcpListener;
use std::net::TcpStream;


fn main()
{
    // Listen for incoming TCP connections on localhost port 7878
    let listener = TcpListener::bind("127.0.0.1:12345").unwrap();

    // Block forever, handling each request that arrives at this IP address
    for stream in listener.incoming()
    {
        let stream = stream.unwrap();

        handle_connection(stream);
    }
}


fn handle_connection(mut stream: TcpStream)
{
    // Read the first 1024 bytes of data from the stream
    let mut buffer = [0; 1024];
    stream.read(&mut buffer).unwrap();

    let get = b"GET / HTTP/1.1";

    // Respond with greetings or a 404,
    // depending on the data in the request
    let (status_line, filename) = if buffer.starts_with(get)
    {
        ("HTTP/1.1 200 OK\r\n", "index.html")
    }
    else
    {
        ("HTTP/1.1 404 NOT FOUND\r\n", "404.html")
    };
    let contents = fs::read_to_string(filename).unwrap();

    // Write response back to the stream,
    // and flush the stream to ensure the response is sent back to the client
    let content_length = contents.len();
    let response = format!("{status_line}Content-Length: {content_length}\r\n\r\n{contents}");
    stream.write_all(response.as_bytes()).unwrap();
    stream.flush().unwrap();
}
