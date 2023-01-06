#pragma once

#include <QObject>
#include <QTcpServer>


class TcpServer : public QObject
{
    Q_OBJECT
public:
    TcpServer(QObject *parent, unsigned short port);
    TcpServer(unsigned short port);

signals:

public slots:
    void newConnection();

protected:
    void initServer(unsigned short port);

private:
    QTcpServer *server;
};
