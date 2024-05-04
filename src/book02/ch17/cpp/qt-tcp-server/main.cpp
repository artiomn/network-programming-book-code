#include <QApplication>
#include <iostream>

#include "tcp_server.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    bool normal_port;
    TcpServer server(QCoreApplication::arguments().at(1).toInt(&normal_port));

    if (!normal_port)
    {
        std::cerr << "Incorrect port!" << std::endl;
        return EXIT_FAILURE;
    }
    return app.exec();
}
