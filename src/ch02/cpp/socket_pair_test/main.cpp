extern "C"
{
#include <sys/socket.h>

// Для read() и write().
#include <unistd.h>
}

#include <cerrno>
#include <iostream>
#include <string>
#include <thread>


const size_t buf_size = 1024;


void child(int socket)
{
    //
    // Эта функция выполняется потомком.
    //

    std::string buf;

    buf.resize(buf_size);

    // Чтение из сокета в Unix-подобных системах может выполняться
    // через read().
    // Сокет здесь представлен обычным файловым дескриптором.
    ssize_t res = read(socket, &buf[0], buf.size());

    if (res < 0)
    {
        // Тут выполняется обработка различных ошибок чтения.
        perror("child read");
        return;
    }

    std::cout << "Child received: '" << buf << "'" << std::endl;

    const std::string msg("Hello parent, I am child");

    // Запись также может быть выполнена через write().
    write(socket, msg.c_str(), msg.size());
    close(socket);
}


void parent(int socket)
{
    //
    // Эта функция выполняется предком.
    //

    std::string buf = "Hello, I'm your parent";

    buf.resize(buf_size);

    ssize_t res = write(socket, buf.c_str(), buf.size());

    if (res < 0)
    {
        // Тут выполняется обработка различных ошибок записи/отправки
        // данных в сокет.
        perror("parent write");
        return;
    }

    // Снова читаю данные.
    res = read(socket, &buf[0], buf.size());

    if (res < 0)
    {
        // Тут выполняется обработка различных ошибок чтения.
        perror("parent read");
        return;
    }

    buf.resize(res);

    std::cout << "Parent received: '" << buf << "'" << std::endl;

    close(socket);
}


int main()
{
    // Массив дескрипторов.
    int fd[2];
    const int parent_socket = 0;
    const int child_socket = 1;
    pid_t pid;

    // Вызываю socketpair(): PF_LOCAL - то же, что и PF_UNIX.
    socketpair(PF_LOCAL, SOCK_DGRAM, 0, fd);

    // Создаю новый процесс.
    pid = fork();

    if (0 == pid)
    {
        // Потомок. Родительский дескриптор больше не потребуется.
        // Нужно его закрыть.
        close(fd[parent_socket]);
        child(fd[child_socket]);
    }
    else if (-1 != pid)
    {
        // Родитель.Дескриптор потомка больше не потребуется.
        // Нужно его закрыть.
        close(fd[child_socket]);
        parent(fd[parent_socket]);
    }
    else
    {
        // Ошибка создания процесса.
        perror("fork");
    }

    return EXIT_SUCCESS;
}
