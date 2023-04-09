// bwpp -O -ml -i=TCPINC -i=INCLUDE example.cpp

//#include <bios.h>
//#include <dos.h>
//#include <io.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "trace.h"
#include "utils.h"
#include "packet.h"
#include "arp.h"
#include "udp.h"
#include "dns.h"
#include "tcp.h"
#include "tcpsockm.h"


char ServerAddrName[80];
uint16_t ServerPort;
// Local port. 0 - if not set.
uint16_t LclPort = 2048;

int8_t Listening = -1;

#define RECV_BUFFER_SIZE (1024)
uint8_t recvBuffer[ RECV_BUFFER_SIZE ];

// Ctrl-Break and Ctrl-C handlers.

volatile uint8_t CtrlBreakDetected = 0;

void __interrupt __far ctrlBreakHandler()
{
  CtrlBreakDetected = 1;
}


void __interrupt __far ctrlCHandler()
{
  // Не делать ничего - Ctrl-C нормальный символ.
}


static void shutdown(int rc)
{
    // Если этого не сделать, операционная система перестанет работать из-за зависшего прерывания таймера.
    Utils::endStack();
    Utils::dumpStats(stderr);
    fclose(TrcStream);
    exit(rc);
}


int main(int argc, char *argv[])
{

    fprintf(stderr, "mTCP Sample program by M Brutman (mbbrutman@gmail.com) (C)opyright 2012-2020\n\n");

    // Прочитать аргументы командной строки.
    parseArgs(argc, argv);

    // Все программы mTCP должны прочитать файл конфигурации mTCP,
    // чтобы узнать, какое прерывание использует пакетный драйвер,
    // каков IP-адрес и сетевая маска, а также некоторые другие
    // необязательные параметры.
    // Все это сделает функция Utils::parseEnv().
    if (Utils::parseEnv() != 0)
    {
        exit(-1);
    }

    // Инициализировать TCP/IP стек.
    // Первый параметр - количество создаваемых TCP сокетов.
    // Второй параметр - количество исходящих TCP-буферов, которые необходимо
    // создать.
    // Параметры три и четыре - пользовательские обработчики Ctrl-Break
    // и Ctrl-C.
    // Если функция возвращает не нуль, это говорит о том, что была ошибка,
    // и программа должна завершиться.
    // Наиболее распространённой ошибкой является невозможность найти пакетный
    // драйвер, поскольку он не был загружен или был загружен с неправильным
    // номером прерывания.
    if (Utils::initStack(2, TCP_SOCKET_RING_SIZE, ctrlBreakHandler, ctrlCHandler))
    {
        fprintf(stderr, "\nFailed to initialize TCP/IP - exiting\n");
        exit(-1);
    }

    // После инициализации mTCP активен и, возможно, получает и обрабатывает
    // пакеты от пакетного драйвера.
    // Теперь надо запустить основной цикл программы, чтобы начать обработку
    // пакетов.
    // mTCP перехватывает прерывание таймера, поэтому, если инициализация
    // прошла, работу стека нужно корректно завершить,
    // вызвав Utils::endStack().

    TcpSocket *mySocket;

    int8_t rc;
    if (0 == Listening)
    {
        //
        // Это программа-клиент.
        //

        fprintf(stdout, "Resolving server address - press Ctrl-Break to abort\n\n");
        IpAddr_t serverAddr;

        // Если был задан числовой IP-адрес, первый вызов
        // Dns::resolve разрешит его, и не придётся ждать.
        // Иначе, нужно в цикле дождаться завершения работы DNS.
        int8_t rc2 = Dns::resolve( ServerAddrName, serverAddr, 1 );
        if (rc2 < 0)
        {
            fprintf(stderr, "Error resolving server\n");
            shutdown(-1);
        }

        uint8_t done = 0;

        while (!done)
        {
            if (CtrlBreakDetected) break;
            if (!Dns::isQueryPending()) break;

            // Цикл должен обрабатывать входящие пакеты, при необходимости
            // повторять запросы ARP и повторять запросы DNS.
            PACKET_PROCESS_SINGLE;
            Arp::driveArp();
            Tcp::drivePackets();
            // Реализация DNS основана на UDP.
            // Dns::drivePendingQuery нужен потому, что пакеты UDP могут быть
            // потеряны, и нужен способ определить, нужно ли нам повторно
            // отправить наш DNS-запрос.
            Dns::drivePendingQuery();
        }

        // Ещё один вызов Dns::resolve вернёт окончательный результат.
        rc2 = Dns::resolve(ServerAddrName, serverAddr, 0);

        if (rc2 != 0)
        {
            fprintf(stderr, "Error resolving server\n");
            shutdown(-1);
        }

        // Выделить сокет.
        // mTCP владеет структурами данных сокета.
        // Пользователь получает указатель на них.
        // Вызов TcpSocketMgr::getSocket() предоставляет сокет нуже для
        // получения указателя.
        // Когда работа будет закончена, его нужно вернуть с помощью
        // вызова TcpSocketMgr::freeSocket().
        mySocket = TcpSocketMgr::getSocket();
        // Установить размер приёмного буфера.
        mySocket->setRecvBuffer(RECV_BUFFER_SIZE);

        fprintf(stderr, "Server resolved to %d.%d.%d.%d - connecting\n\n",
                serverAddr[0], serverAddr[1], serverAddr[2], serverAddr[3]);

        // Выполнить неблокирующее соединение, ожидать 10 секунд перед тем,
        // как выдать ошибку.
        rc = mySocket->connect(LclPort, serverAddr, ServerPort, 10000);
    } // if (0 == Listening) ...
    else
    {
        //
        // Это - сервер.
        //

        fprintf(stderr, "Waiting for a connection on port %u. Press [ESC] to abort.\n\n", LclPort);

        TcpSocket *listeningSocket = TcpSocketMgr::getSocket();
        // Вызов listen сообщает mTCP, какой порт прослушивать
        // и какой размер приёмного буфера использовать для вновь
        // созданных сокетов.
        listeningSocket->listen(LclPort, RECV_BUFFER_SIZE);

        // Listen не блокирует выполнение. Надо подождать.
        while (1)
        {
            if (CtrlBreakDetected)
            {
                // Пользователь нажал Ctrl+C.
                rc = -1;
                break;
            }

            PACKET_PROCESS_SINGLE;
            Arp::driveArp();
            Tcp::drivePackets();

            // TcpSocketMgr::accept() вернёт указатель на сокет для обмена
            // с клиентом, после его подключения.
            mySocket = TcpSocketMgr::accept();
            if (mySocket != NULL)
            {
                // Прослушивающий сокет больше не требуется.
                listeningSocket->close();
                TcpSocketMgr::freeSocket(listeningSocket);
                rc = 0;
                break;
            }

            if (_bios_keybrd(1) != 0)
            {
                char c = _bios_keybrd(0);
                if ((c == 27) || (c == 3))
                {
                    rc = -1;
                    break;
                }
            }
        } // while(1)
    } // Listening != 0

    // Цикл обработки пакетов.
    while (programIsNotEnding)
    {
        // Обработка входящих пакетов.
        // PACKET_PROCESS_SINGLE - это макрос, проверяющий наличие новых
        // пакетов от драйвера пакетов.
        // Если таковые обнаружены, макрос обрабатывает эти пакеты.
        PACKET_PROCESS_SINGLE;
        // Arp::driveArp() используется для проверки ожидающих запросов ARP
        // и повторения их при необходимости.
        Arp::driveArp();
        // Tcp::drivePackets() используется для отправки пакетов, которые были
        // поставлены в очередь для отправки.
        Tcp::drivePackets();

        if (mySocket->isRemoteClosed())
        {
            // Сокет был закрыт удалённой стороной.
            done = 1;
        }

        // Если сокет не закрыт, то код попытается получить на него данные.
        // Код возврата 0 указывает на отсутствие данных.
        // Отрицательный код возврата указывает на ошибку сокета,
        // а положительный код возврата - количество полученных байтов.
        int16_t recvRc = mySocket->recv(recvBuffer, RECV_BUFFER_SIZE);

        if (recvRc > 0)
        {
            // Любые полученные байты сразу же записываются в stdout.
            write(1, recvBuffer, recvRc);
        }
        else if (recvRc < 0)
        {
            fprintf(stderr, "\nError reading from socket\n");
            done = 1;
        }

        // Чтение и отправка пользовательского ввода.

        if (CtrlBreakDetected)
        {
            fprintf(stderr, "\nCtrl-Break detected\n");
            done = 1;
        }

        if (_bios_keybrd(1))
        {
            // Если пользователь нажал клавишу, нужно прочитать её
            // с клавиатуры и обработать соответствующим образом.
            uint16_t key = _bios_keybrd(0);
            char ch = key & 0xff;

            if (0 == ch)
            {
                uint8_t ekey = key >> 8;

                if (45 == ekey)
                {
                    // Alt-X - выход.
                    done = 1;
                }
                else if ( 35 == ekey)
                {
                    // Alt-H - справка.
                    fprintf(stderr, "\nSample: Press Alt-X to exit\n\n");
                }
            }
            else
            {
                // Любой символ отправляется в сеть.
                int8_t sendRc = mySocket->send((uint8_t *)&ch, 1);
                // Пакет не будет отправлен в сеть, пока не вызван
                // Tcp::drivePackets().
                // Этот вызов также обрабатывает повторную отправку пакета,
                // если он потерялся.
            }
        } // Нажатие клавиши.
    } // Цикл обработки пакетов.
} // main
