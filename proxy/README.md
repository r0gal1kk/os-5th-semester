HTTP Proxy Server (C, POSIX Threads, picohttpparser)

## Описание

Данный проект — многопоточный HTTP-прокси сервер, реализованный на языке C с использованием POSIX сокетов и библиотеки **picohttpparser** для разбора HTTP-заголовков.

Прокси принимает входящие HTTP-соединения, извлекает заголовок `Host`, устанавливает соединение с целевым сервером и прозрачно пересылает данные между клиентом и сервером.

---

# Возможности

* Поддержка HTTP/1.0 и HTTP/1.1 запросов
* Многопоточность (один поток на клиента)
* Корректный разбор HTTP-заголовков с picohttpparser
* Защита от частичного TCP-чтения (stream parsing)
* Полный duplex-relay (client ↔ server) через `poll()`
---

# Сборка

Требования:

* Linux
* gcc / clang
* pthreads
* picohttpparser (в проекте как `picohttpparser.h` + `picohttpparser.c`)

Сборка:

```bash
make
```

---

# Запуск

Требуется root (порт 80):

```bash
sudo ./proxy-http
```

Или поменять `LISTEN_PORT` на 8080.

После запуска:

```text
HTTP proxy listening on port 80
```

---

# Примеры запросов

## Через curl

```bash
curl -v -x localhost:80 http://gramota.ru/
```

---

# Как работает

## 1. Создание listening-сокета

Функция `create_listen_socket()`:

* создаёт TCP-сокет
* включает `SO_REUSEADDR`
* делает `bind()` и `listen()`

---

## 2. Принятие клиентов

Главный поток:

* вызывает `accept()`
* для каждого клиента создаёт отдельный `pthread`

---

## 3. Чтение и разбор HTTP запроса

В `client_thread()`:

* данные читаются **в цикле**, пока picohttpparser не скажет, что запрос полностью получен
* учитывается TCP fragmentation (заголовки могут прийти частями)
* если запрос слишком большой → 413

---

## 4. Поиск заголовка Host

Используется функция:

```c
find_header(headers, num_headers, "Host");
```

Она гарантирует, что найден именно заголовок Host (а не первый header).

---

## 5. Соединение с целевым сервером

Функция `connect_to_host()`:

* использует `getaddrinfo()`
* перебирает адреса
* подключается через `connect()`

---

## 6. Прокси-пересылка данных

Функция `proxy_relay()`:

* использует `poll()` для duplex-I/O
* клиент → сервер
* сервер → клиент
* корректно обрабатывает partial send/recv
* завершает соединение при ошибках или EOF

---

# Тестируемые сайты

Примеры HTTP-сайтов для проверки:

```text
http://gramota.ru/
http://parallels.nsu.ru/WackoWiki/KursOperacionnyeSistemy/PraktikumPosixThreads/
http://www.realtimerendering.com/blog/
http://www.gutenberg.org/ebooks/2600.txt.utf-8
http://68k.news/
http://rinkworks.com/
http://ascii.textfiles.com/
http://theoldnet.com/
http://www.midwinter.com/
```

---
