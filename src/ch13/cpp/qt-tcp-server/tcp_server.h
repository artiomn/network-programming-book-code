#pragma once

#include <QObject>
#include <QTcpServer>


class TcpServer : public QObject
{
    Q_OBJECT
public:
    TcpServer(QObject *parent, unsigned short port);
    explicit TcpServer(unsigned short port);

signals:

    // cppcheck-suppress unknownMacro
public slots:
    void newConnectign();

protected:
    void initServer(unsigned short port);

private:
    QTcpServer *server;
};
