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

public slots:  // cppcheck-suppress unknownMacro
    void newConnection();

protected:
    void initServer(unsigned short port);

private:
    QTcpServer *server;
};
