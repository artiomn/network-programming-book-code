use std::fs;
use std::net::SocketAddr;
use tokio::net::TcpListener;
use tokio::io::{AsyncReadExt, AsyncWriteExt};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>>
{
    let addr = SocketAddr::from(([127, 0, 0, 1], 12345));
    let listener = TcpListener::bind(&addr).await?;

    println!("Listening on: http://{}", addr);

    loop
    {
        let (mut stream, _) = listener.accept().await?;

        tokio::spawn(async move {
            let mut buffer = [0; 1024];
            let _ = stream.read(&mut buffer).await;

            let get = b"GET / HTTP/1.1";

            let (status_line, filename) = if buffer.starts_with(get)
            {
                ("HTTP/1.1 200 OK\r\n", "index.html")
            }
            else
            {
                ("HTTP/1.1 404 NOT FOUND\r\n", "404.html")
            };
            let contents = fs::read_to_string(filename).unwrap();

            let content_length = contents.len();
            let response = format!("{status_line}Content-Length: {content_length}\r\n\r\n{contents}");
            let _ = stream.write_all(response.as_bytes()).await;
        });
    }
}
