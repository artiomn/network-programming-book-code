#include <QDebug>
#include <QNetworkInterface>


int main(int argc, char *argv[])
{
    for (const auto &i : QNetworkInterface::allInterfaces())
    {
        qDebug() << i.index() << ":" << i.name() << " " << i.hardwareAddress();
        for (const auto &a : i.addressEntries()) qDebug() << "  " << a.ip().toString() << "/" << a.netmask().toString();
    }

    return EXIT_SUCCESS;
}
