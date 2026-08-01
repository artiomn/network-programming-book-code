#include "tcp_server.h"

#include <QDebug>
#include <QTcpSocket>


TcpServer::TcpServer(QObject *parent, unsigned short port) : QObject(parent)
{
    initServer(port);
}


TcpServer::TcpServer(unsigned short port) : QObject(nullptr)
{
    initServer(port);
}


void TcpServer::initServer(unsigned short port)
{
    server = new QTcpServer(this);

    // When user connects, signal will be emitted.
    connect(server, &QTcpServer::newConnection, this, &TcpServer::newConnection);

    if (!server->listen(QHostAddress::Any, port))
    {
        qDebug() << "Server could not start!";
    }
    else
    {
        qDebug() << "Server started on port" << port << "...";
    }
}


void TcpServer::newConnection()
{
    QTcpSocket *client_socket = server->nextPendingConnection();

    qDebug() << "New connection from" << client_socket->peerAddress() << ":" << client_socket->peerPort();

    client_socket->write("Qt TCP server\r\n");
    client_socket->flush();

    connect(
        client_socket, &QTcpSocket::readyRead,
        [client_socket]() { qDebug() << "Client send data:" << client_socket->readLine(); });

    client_socket->waitForReadyRead(10000);

    qDebug() << "Close connection" << client_socket->peerAddress() << ":" << client_socket->peerPort();

    client_socket->close();
}
