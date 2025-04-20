# Repository with a code for the "Network Programming" book

This repository contains the programming examples for the book "Network Programming"

## Table of contents

- [Beginning](#beginning)
- [FAQ](#faq)
  - [Is here all code without bugs?](is-here-all-code-without-bugs)
  - [Why am I banned?](#why-am-i-banned)
  - [Do I need update?](#do-i-need-update)
  - [How to build examples?](#how-to-build-examples)
  - [What to do if build failed?](#build-failed)
  - [Why do you use Linux?](#why-do-you-use-linux)
  - [Can I build the examples under Windows?](#can-i-build-the-examples-under-windows)
  - [What can I read about CMake?](#what-can-i-read-about-cMake)
  - [Docker - what's it?](#docker---what-s-it)
    - [How to use Docker?](#how-to-use-docker)
    - [Why can I run build only under root user?](#why-can-i-run-build-only-under-root-user)
    - [Can I build in the IDE?](#can-i-build-in-the-ide)
    - [I can't connect to Docker or IDE doesn't see this?](#i-can-t-connect-to-docker-or-ide-doesn-t-see-this)
    - [Something absent in the build image](#something-absent-in-the-build-image)
    - [Can I damage my system, when I works in the Docker environment?](can-i-damage-my-system--when-i-works-in-the-docker-environment)
  - [How to run examples?](#how-to-run-examples)
  - [How to run console?](#how-to-run-console)
  - [Where to download Netcat under Windows?](where-to-download-netcat-under-windows)
  - [I'm doing all right, but something is not connected](#i-m-doing-all-right--but-something-is-not-connected)
  - [I've found error in the code, how can I fix it?](#i-ve-found-error-in-the-code--how-can-i-fix-it)
- [For contributors](#for-contributors)
- [Authors contacts](#authors-contacts)

## Beginning

To start work:

- If [Docker](https://www.docker.com/) was not installed, [install it](#how-to-use-docker).
- Если [Git](https://git-scm.com/) не установлен в системе, [установите его](https://git-scm.com/book/ru/v2/%D0%92%D0%B2%D0%B5%D0%B4%D0%B5%D0%BD%D0%B8%D0%B5-%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0-Git).
- Clone repository: `git clone https://github.com/artiomn/network-programming-book-code.git`.
  **Attention: clone repository in the directory with path contains only latin symbols!**
- Go to directory `network-programming-book-code`.
- Run script `./build_dockerized.sh`

If this is the first run, build will consume significant time.

To run QtCreator in the directory `network-programming-book-code` run the following command: `./run -q`.
IDE will started, then you need to open file as a project `CMakeLists.txt` in the directory `/home/developer/src` or `/usr/src/gb/src`.
The first directory is a symlink.

## FAQ

### Is here all code without bugs?

Наоборот. Почти наверняка здесь есть ошибки.
Книга - большой проект. Их появление неизбежно. Поэтому, если вы заметили ошибку, [смело заводите Issue](https://github.com/artiomn/network-programming-book-code/issues).
Аналогично вы можете заводить issue с ошибками в тексте книги, и если потребуется, будет выпущен соответствующий erratum.
Предложения также приветствуются.

[ToC ⮐](#table-of-contents)

### Why am I banned?

Есть некоторые очевидные для большинства людей вещи, которые не к месту в репозитории кода для книги.
Например, политика. Хотите донести своё мнение? Сделайте это в чате Telegram канала. Там мы забаним вас гораздо быстрее.

Помимо очевидного, есть то, что не все **начинающие** разработчики понимают.
Например:

- Попытка реквестов с решениями заданий в этот репозиторий. После этого вы будете заблокированы немедленно. Без персональных объяснений, почему так делать нельзя.
- Попытка "наездов" с претензиями к авторам, которые "вам должны". Это больше относится к очевидным для большинства людей вещам. Мы вам продаём книгу: вы платите, магазин вам её предоставляет. На этом наш договор оканчивается, и мы вам больше не должны ничего (в т.ч. реклама не является публичной офертой). Всё остальное делается по доброй воле авторов.

[ToC ⮐](#table-of-contents)

### Do I need update?

Иногда - надо. В код или образ могут вноситься изменения.
Если вы работаете с Git репозиторием, код обновляется следующим образом:

```
git pull
```

Docker образы могут быть обновлены следующими командами:

```
artiomn/gb-build-image
artiomn/gb-qt-creator-image
```

[ToC ⮐](#table-of-contents)

### How to build examples?

Сборка производится, используя CMake, поэтому:

- Вы можете использовать любую современную IDE для сборки.
- Скрипт `./build.sh`.
- Прямой запуск CMake.

Сборка не подгружает зависимости автоматически.
Поэтому, для того, чтобы собрать без необходимости заниматься установкой множества пакетов, используется подготовленная среда.
Она представляет собой Docker-контейнер, образ которого [лежит на docker.hub](https://hub.docker.com/r/artiomn/gb-build-image).
Сборка в ней запускается через скрипт `./build_dockerized.sh`.

Под Windows сборка была проверена на MSVS 2019 и собранные артефакты будут находиться в `src\out\build\windows-default\bin`.
Собираться под Windows будет не всё, есть примеры только под Linux.

[ToC ⮐](#table-of-contents)

### Build failed

Может быть несколько причин из-за которых не проходит сборка.
Для начала, убедитесь, что у вас:

- Правильно [настроен и работает Docker](#%D0%BA%D0%B0%D0%BA-%D0%B8%D1%81%D0%BF%D0%BE%D0%BB%D1%8C%D0%B7%D0%BE%D0%B2%D0%B0%D1%82%D1%8C-docker).
- Если это первая сборка, есть доступ в Интернет.

Далее, проверьте, что каталог в котором производится сборка, не содержит в пути не латинских символов, пробелов и спецсимволов.

**Не делайте каталог вида**: `~/Общедоступные/user/geekbrains/lessons/C++-network/Programming_Lessons & Code/Репозиторий!/*С кодом?/network-programming-book-code`.
Для **большинства сборок** - это верный путь к проблемам, о которых не знали авторы книги и разработчики CMake.

Положите репозиторий в каталог с нормальным именем, например `~/projects/cpp-network-tasks`.

Если вы уже собирали проект локально и хотите собрать его в Docker или, наоборот, собирали в Docker и хотите собрать локально, CMake выдаст ошибку, похожую на следующую:

```
CMake Error: The current CMakeCache.txt directory /home/user/projects/cpp-network-tasks/build/CMakeCache.txt is different than the directory /usr/src/gb/build where CMakeCache.txt was created. This may result in binaries being created in the wrong place. If you are not sure, reedit the CMakeCache.txt
CMake Error: The source "/home/artiom/user/cpp-network-tasks/src/CMakeLists.txt" does not match the source "/usr/src/gb/src/CMakeLists.txt" used to generate cache.  Re-run cmake with a different source directory.
```

Что говорит о том, что конфигурации CMake различаются, вы запускаете его в разном окружении.

Чтобы исправить это, **удалите каталог `build`** и запустите сборку заново:

```
➭ rm -rf build && ./build_dockerized.sh
```

[ToC ⮐](#table-of-contents)

### Why do you use Linux?

- Потому что, на Linux и BSD системах работают большинство сетевых приложений.
- На Linux работает автор курсов.
- Кроме Linux есть множество других ОС, и рассмотреть особенности каждой невозможно.
- Перевод кода на Windows оговорён.
- По возможности, код и так кроссплатформенный.

[ToC ⮐](#table-of-contents)

### Can I build the examples under Windows?

Да, возможно собрать часть кода. Сборка проверялась на MS Visual Studio 2019.
Чтобы собрать код, надо открыть корневой CMakeLists.txt, как CMake проект.

Но есть следующие проблемы:

- Есть код специфичный для Linux, например перехватчик вызовов, который будет собираться и работать только на этой платформе.
- Некоторый код просто не был адаптирован для Windows, и его сборка выключена.
- Есть редкие примеры, которые собираются, но работать корректно не будут (один из таких - асинхронный сервер на `select()`).
  Оно не работает, потому что не было достаточно времени и мотивации, чтобы доработать под Windows.
  Если вы считаете, что можете доделать такой код - You're welcome.
- В Windows есть не все библиотеки, а CMake не имеет, например модуля для поиска Qt, если Qt не установлен.
  Это приводит к тому, что пример не просто нельзя собрать, если чего-то не хватает, а падает сборка.
  Конечно, возможно это исправить, если вы считаете, что нужно, репозиторий открыт для правок.

Также, есть некоторый код, специфичный для Windows, но в уроках он, как правило, не упоминается.

[ToC ⮐](#table-of-contents)

### What can I read about CMake?

Есть:

- [Уроки на Youtube](https://www.youtube.com/watch?v=SM3Klt2rY8g&list=PL6x9Hnsyqn2UwWjSvjCzAY6sEOBrHY7VH).
- [Документация](https://cmake.org/documentation/).
- [Статьи](https://habr.com/ru/post/431428/).
- Много информации в Интернете.

[ToC ⮐](#table-of-contents)

### Docker - what's it?

Предполагается, что в процессе работы с книгой, вы будете использовать поиск в Интернет.
Соответственно, [Google поможет](https://google.gik-team.com/?q=Docker).
Если очень кратко, [Docker](https://ru.wikipedia.org/wiki/Docker) - это один из вариантов реализации автоматизации инфраструктуры контейнеризации.
Он позволяет изолировать приложения в контейнерах, образы которых скачаны из репозитория.
Здесь он нужен для того, чтобы:

- Вы могли собрать код, не устанавливая лишних библиотек.
- Вы могли проверить сборку тем же самым компилятором, что и проверяющий.

Напомню, что образ с инструкциями [лежит на Docker.hub](https://hub.docker.com/r/artiomn/gb-build-image).

[ToC ⮐](#table-of-contents)

#### How to use Docker?

На Windows только [вместе с Linux подсистемой](https://docs.microsoft.com/en-us/windows/wsl/install).
На Linux его надо установить, а как, зависит от вашего дистрибутива.
Например, в deb-based это делается [по следующей инструкции](https://docs.docker.com/engine/install/debian/).

[ToC ⮐](#table-of-contents)

#### Why can I run build only under root user?

Управлять Docker, в том числе и запускать или останавливать контейнеры могут только:

- Пользователь `root`.
- Пользователи в группе `docker`.

Добавить вашего пользователя в группу Docker возможно по-разному. Один из вариантов, который будет работать для многих дистрибутивов:

```
$ sudo usermod -a -G docker "${USER}"
```

После этого надо завершить сессию, например выйти на Login Screen, и снова войти.

[ToC ⮐](#table-of-contents)

#### Can I build in the IDE?

Это зависит от IDE. Возможно использовать удалённую сборку по SSH (сервер установлен в образе), некоторые IDE, такие как CLion, поддерживают работу с Docker напрямую.

[ToC ⮐](#table-of-contents)

##### CLion

Пример настройки CLion показан [здесь](https://blog.jetbrains.com/clion/2021/10/clion-2021-3-eap-new-docker-toolchain/).

Настройки конфигурации Docker показаны на скриншоте:

![](img/todo/clion_docker_toolchain.png)

Настройки конфигурации CMake:

![](img/todo/clion_docker_cmake.png)

[ToC ⮐](#table-of-contents)

##### QtCreator

Раньше QtCreator не поддерживал [удалённую сборку по SSH](https://stackoverflow.com/questions/42880004/with-qtcreator-how-can-i-build-my-project-on-a-remote-server-i-have-ssh-access).

Есть вариант настройки [с подмонтированием каталога через SSHFS](https://forum.qt.io/topic/9928/qt-creator-remote-development-via-ssh-for-desktop-projects).
Но все пути к инклудам, а также библиотекам, естественно, будут некорректны, т.к. в контейнере они другие.

Возможно самый просто вариант - установить QtCreator в контейнер и запустить оттуда.
На данный момент для этого [собран образ](https://hub.docker.com/r/artiomn/gb-qt-creator-image).
Чтобы запустить его, используйте:

```
./run -q
```

![QtCreator, запущенный в Docker](img/todo/qt_creator_docker.png)

[ToC ⮐](#table-of-contents)

#### I can't connect to Docker or IDE doesn't see this

Проверьте:

- Установлен ли у вас Docker. Если нет - [установите](https://docs.docker.com/engine/install/).
- Достаточно ли прав у вашего пользователя, что с Docker взаимодействовать.

В большинстве дистрибутивов, при установке Docker создаёт группу `docker`, что вы можете проверить, выполнив следующую команду:

```
$ grep docker /etc/group
docker:x:997:
```

Видно, что пользователя в этой группе нет.
Добавьте его туда:

```
$ sudo usermod -a -G docker "$USER"
```

```
$ grep docker /etc/group
docker:x:997:artiom
```

После того, как вы перелогинитесь в системе, группа будет видна:

```
$ groups
sys network power video storage lp input audio wheel artiom docker
```

[ToC ⮐](#table-of-contents)

#### Something absent in the build image

Возможно, образ устарел.
Обновите его:

```
$ docker pull artiomn/gb-build-image
Using default tag: latest
latest: Pulling from artiomn/gb-build-image
Digest: sha256:be8e76c39543bd03f503a294d96dc578c5ad90b77c1473e6b03ef1feb88c0b0c
Status: Downloaded newer image for artiomn/gb-build-image:latest
docker.io/artiomn/gb-build-image:latest
```

[ToC ⮐](#table-of-contents)

### Can I damage my system, when I works in the Docker environment?

Можете. Контейнер запускается в privileged режиме. Т.е. из него возможно создавать устройства, а значит испортить всё, что угодно.
Это требуется для того, чтобы сети, в частности Yggdrasil, могли создать TUN устройство.

[ToC ⮐](#table-of-contents)

### How to run examples?

Если вы производили сборку, как указано выше, собранные бинарные файлы будут находиться в каталоге `build/bin`.
Запустить вы их можете напрямую, однако из-за отсутствующих в системе зависимостей работать может не всё.
Поэтому, запуск также производится в контейнере, используя команду `./run`, которой передаются необходимые приложению аргументы и путь к нему.

Пример:

```
➭ ./run sudo ./build/bin/ch02-ping google.com
Pinging "google.com" [64.233.165.113]
Start to sending packets...
Sending packet 0 to "google.com" request with id = 29
Receiving packet 0 from "google.com" response with id = 29, time = 15.5ms
```

[ToC ⮐](#table-of-contents)

### How to run console?

- `./run`
- `docker-compose run --rm gb` - для тех, кто пользуется docker-compose.

**Обратите внимание:** консоль запускается через **скрипт `./run`** в корне репозитория, **не** через `docker run`.

[ToC ⮐](#table-of-contents)

### Where to download Netcat under Windows?

Например, [здесь](https://github.com/diegocr/netcat).

**Attention!**
Windows Defender может заблокировать Netcat, определив его, как Netcat.Hacktool.
В таком случае, просто отключите "защиту реального времени".

[ToC ⮐](#table-of-contents)

### I'm doing all right, but something is not connected

Такая проблема была у одного из студентов. Он выяснил, что Netcat подключался на IPv6 адрес, тогда как сервер прослушивал только IPv4.
Как исправить? Зависит от приложения. Задайте ему не доменное имя, а IPv4 адрес явно, при возможности.
Для Netcat возможно использовать опцию `-4` (не все реализации её поддерживают).

[ToC ⮐](#table-of-contents)

### I've found error in the code, how can I fix it?

Да. Следуя обычному процессу Guthub:

- Сначала делаете форк репозитория.
- Клонируете его, исправляете ошибку.
- Делаете Pull request.

Или просто можете просто завести [Issue](https://github.com/artiomn/cpp-network-tasks/issues/new/choose).
Но, в этом случае, обещать быстрое исправление я не могу.

[ToC ⮐](#table-of-contents)

## For contributors

В корневой директории имеется скрипт `install_git_hooks.py`, данный скрипт проверяет наличие установленного инструмента pre-commit, и, если его нет, устанавливает его. pre-commit - это инструмент для автоматизации проверок и тестов перед фиксацией (коммитом) изменений в репозитории Git. Он позволяет определить набор проверок и скриптов, которые должны быть выполнены перед тем, как изменения будут зафиксированы в репозитории. Использование pre-commit помогает поддерживать качество кода, уменьшает вероятность внесения ошибок и упрощает совместную работу.

Для самостоятельной установке необходимо открыть командную строку и ввести:

```cmd
pip3 install pre-commit
```

После чего перейти в корневой репозиторий `network-programming-book-code` и ввести команду:

```cmd
 pre-commit install --install-hooks
```

Затем

```cmd
pre-commit run
```

Эта команда запускает все тесты необходимые для коммита.

Для правильной работы pre-commit и проверки cpp файлов необходимо также установить [cppcheck](https://github.com/danmar/cppcheck#visual-studio), инструкция по настройке есть на самой странице, так же в [данном](https://cppcheck.sourceforge.io/manual.pdf) мануале.

[ToC ⮐](#table-of-contents)

## Authors contacts

Если у вас есть, что сказать, [заведите Issue](https://github.com/artiomn/network-programming-book-code/issues).
Либо обратитесь в [Telegram канал](https://t.me/net_progr) по книге.

[ToC ⮐](#table-of-contents)
