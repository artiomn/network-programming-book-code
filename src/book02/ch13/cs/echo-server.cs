using System;
using System.Net;
using System.Net.Sockets;
using System.Threading.Tasks;


namespace AsyncConsoleApp
{

class EchoServer
{
    // cppcheck-suppress unusedPrivateFunction
    public async Task Run(short port)
    {
        TcpListener tcp_server = new TcpListener(IPAddress.Any, port);
        try
        {
            tcp_server.Start();
            Console.WriteLine("Server started... ");

            while (true)
            {
                var client_sock = await tcp_server.AcceptSocketAsync();
                Console.WriteLine($"New connection: {client_sock.RemoteEndPoint}");
                ProcessClient(client_sock);
            }
        }
        finally
        {
            tcp_server.Stop();
        }
    }

    private async void ProcessClient(Socket client_sock)
    {
        var ep = client_sock.RemoteEndPoint;
        try
        {
            var buf = new byte[1024];

            while (await client_sock.ReceiveAsync(buf, SocketFlags.None) != 0)
            {
                await client_sock.SendAsync(buf, SocketFlags.None);
            }
        }
        catch (System.Net.Sockets.SocketException e)
        {
            Console.WriteLine($"Client disconnected {e}");
        }
        finally
        {
            client_sock.Close();
            Console.WriteLine($"Client disconnected {ep}");
        }
    }
}


class Program
{
    // cppcheck-suppress unusedPrivateFunction
    public static int Main(string[] args)
    {
        var server = new EchoServer();
        server.Run(12345).Wait();
        return 0;
    }
}

}
