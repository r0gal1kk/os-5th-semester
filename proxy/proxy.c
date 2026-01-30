#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include "picohttpparser.h"

#define LISTEN_PORT 80
#define BUF_SIZE 8192
#define MAX_HEADERS 32

typedef struct {
    int client_fd;
} client_arg_t;

static int create_listen_socket(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        return -1;

    if (listen(fd, SOMAXCONN) < 0)
        return -1;

    return fd;
}

static int connect_to_host(const char *host, const char *port) {
    struct addrinfo hints, *res, *it;
    int fd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &res) != 0)
        return -1;

    for (it = res; it; it = it->ai_next) {
        fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (fd < 0)
            continue;

        if (connect(fd, it->ai_addr, it->ai_addrlen) == 0)
            break;

        close(fd);
        fd = -1;
    }

    freeaddrinfo(res);
    return fd;
}

typedef struct {
    const char *value;
    size_t value_len;
} header_value_t;

static header_value_t find_header(
    struct phr_header *headers,
    size_t num_headers,
    const char *name
) {
    header_value_t res = { NULL, 0 };

    for (size_t i = 0; i < num_headers; i++) {
        if (headers[i].name_len == strlen(name) &&
            strncasecmp(headers[i].name, name, headers[i].name_len) == 0) {

            res.value = headers[i].value;
            res.value_len = headers[i].value_len;
            return res;
        }
    }
    return res;
}

static void proxy_relay(int client_fd, int server_fd) {
    struct pollfd fds[2];
    char buffer[BUF_SIZE];
    int stop = 0;

    fds[0].fd = client_fd;     
    fds[0].events = POLLIN;
    fds[1].fd = server_fd;     
    fds[1].events = POLLIN;

    while (!stop) {
        int poll_result = poll(fds, 2, -1);
        if (poll_result < 0) {
            if (errno == EINTR) continue;
            stop = 1;
            continue;
        }
        if (poll_result == 0) {
            stop = 1;
            continue;
        }

        // клиент -> сервер
        if (!stop && (fds[0].revents & POLLIN)) {
            ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                stop = 1;
                continue;
            }

            size_t remaining = (size_t)bytes_received;
            size_t sent_so_far = 0;

            while (sent_so_far < remaining && !stop) {
                ssize_t sent_this_time = send(server_fd, buffer + sent_so_far, remaining - sent_so_far, 0);

                if (sent_this_time <= 0) {
                    stop = 1;
                } else {
                    sent_so_far += (size_t)sent_this_time;
                }
            }
        }

        // сервер -> клиент
        if (!stop && (fds[1].revents & POLLIN)) {
            ssize_t bytes_received = recv(server_fd, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                stop = 1;
                continue;
            }

            size_t remaining = (size_t)bytes_received;
            size_t sent_so_far = 0;

            while (sent_so_far < remaining && !stop) {
                ssize_t sent_this_time = send(client_fd, buffer + sent_so_far, remaining - sent_so_far, 0);

                if (sent_this_time <= 0) {
                    stop = 1;
                } else {
                    sent_so_far += (size_t)sent_this_time;
                }
            }
        }

        if ((fds[0].revents & (POLLHUP | POLLERR)) ||
            (fds[1].revents & (POLLHUP | POLLERR))) {
            stop = 1;
        }
    }
}

static void *client_thread(void *arg) {
    client_arg_t *ctx = arg;
    int client_fd = ctx->client_fd;
    free(ctx);

    char buf[32768];
    size_t buf_used = 0;

    struct phr_header headers[MAX_HEADERS];
    size_t num_headers = MAX_HEADERS;

    const char *method, *path;
    int minor_version;
    size_t method_len, path_len;

    int pret;

    // Читаем заголовки до тех пор, пока не распарсим полностью
    while (1) {
        ssize_t r = recv(client_fd, buf + buf_used, sizeof(buf) - buf_used - 1, 0);

        if (r <= 0) {
            close(client_fd);
            return NULL;
        }

        buf_used += r;

        pret = phr_parse_request(buf, buf_used, &method, 
            &method_len, &path, &path_len, 
            &minor_version, headers, &num_headers, 0);

        if (pret > 0) {
            //success
            break;
        }

        if (pret == -1) {
            // некорректный запрос
            const char *msg = "HTTP/1.0 400 Bad Request\r\n\r\n";
            send(client_fd, msg, strlen(msg), 0);
            close(client_fd);
            return NULL;
        }

        if (pret == -2) {
            // нужно больше данных
            if (buf_used >= sizeof(buf) - 1024) {
                const char *msg = "HTTP/1.0 413 Request Entity Too Large\r\n\r\n";
                send(client_fd, msg, strlen(msg), 0);
                close(client_fd);
                return NULL;
            }
            // продолжаем чтение
            continue;
        }
    }

    header_value_t host_hdr = find_header(headers, num_headers, "Host");
    if (!host_hdr.value) {
        const char *msg = "HTTP/1.0 400 Bad Request\r\n\r\nNo Host header";
        send(client_fd, msg, strlen(msg), 0);
        close(client_fd);
        return NULL;
    }

    char host_buf[256];
    snprintf(host_buf, sizeof(host_buf), "%.*s",
             (int)host_hdr.value_len, host_hdr.value);

    int server_fd = connect_to_host(host_buf, "80");
    if (server_fd < 0) {
        const char *msg = "HTTP/1.0 502 Bad Gateway\r\n\r\n";
        send(client_fd, msg, strlen(msg), 0);
        close(client_fd);
        return NULL;
    }

    send(server_fd, buf, pret, 0);

    proxy_relay(client_fd, server_fd);

    close(server_fd);
    close(client_fd);
    return NULL;
}

int main(void) {
    int listen_fd = create_listen_socket(LISTEN_PORT);
    if (listen_fd < 0) {
        perror("listen");
        return 1;
    }

    printf("HTTP proxy listening on port %d\n", LISTEN_PORT);

    while (1) {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd < 0)
            continue;

        client_arg_t *arg = malloc(sizeof(*arg));
        arg->client_fd = client_fd;


        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, arg) != 0) {
            close(client_fd);
            free(arg);
        }
        pthread_attr_destroy(&attr);
    }
}
