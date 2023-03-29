#include <iostream>
#include <memory>
#include <string>

#include <QObject>
#include <QDebug>
#include <QTcpSocket>
#include <QTcpServer>


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        qDebug() << "Incorrect port!";
        return EXIT_FAILURE;
    }

    auto port = std::stoi(argv[1]);

    auto server{std::make_unique<QTcpServer>()};

    // When user connects, signal will be emitted.
    QObject::connect(server.get(), &QTcpServer::newConnection, [&server]()
    {
        QTcpSocket *client_socket = server->nextPendingConnection();

        qDebug()
            << "New connection from"
            << client_socket->peerAddress() << ":"
            << client_socket->peerPort();

        client_socket->write("Qt TCP server\r\n");
        client_socket->flush();

        QObject::connect(client_socket, &QTcpSocket::readyRead, [client_socket]()
        {
            qDebug()
               << "Client send data:"
               << client_socket->readLine();
        });

        client_socket->waitForReadyRead(10000);

        qDebug()
            << "Close connection"
            << client_socket->peerAddress() << ":"
            << client_socket->peerPort();

        client_socket->close();
    });

    if (!server->listen(QHostAddress::Any, port))
    {
        qDebug() << "Server could not start!";
        return EXIT_FAILURE;
    }
    else
    {
        qDebug() << "Server started on port" << port << "...";
    }

    while (true)
    {
        server->waitForNewConnection();
    }
}
