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

static const char *find_header(
    struct phr_header *headers,
    size_t num_headers,
    const char *name
) {
    for (size_t i = 0; i < num_headers; i++) {
        if (strncasecmp(headers[i].name, name, headers[i].name_len) == 0)
            return headers[i].value;
    }
    return NULL;
}

static void proxy_relay(int a, int b) {
    struct pollfd fds[2];
    char buf[BUF_SIZE];

    fds[0].fd = a;
    fds[0].events = POLLIN;

    fds[1].fd = b;
    fds[1].events = POLLIN;

    while (1) {
        int r = poll(fds, 2, -1);
        if (r <= 0)
            break;

        if (fds[0].revents & POLLIN) {
            ssize_t n = recv(a, buf, sizeof(buf), 0);
            if (n <= 0) break;
            send(b, buf, n, 0);
        }

        if (fds[1].revents & POLLIN) {
            ssize_t n = recv(b, buf, sizeof(buf), 0);
            if (n <= 0) break;
            send(a, buf, n, 0);
        }
    }
}

static void *client_thread(void *arg) {
    client_arg_t *ctx = arg;
    int client_fd = ctx->client_fd;
    free(ctx);

    pthread_detach(pthread_self());

    char buf[BUF_SIZE];
    struct phr_header headers[MAX_HEADERS];
    size_t num_headers = MAX_HEADERS;

    const char *method, *path;
    int minor_version;
    size_t method_len, path_len;

    ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
    if (n <= 0) {
        close(client_fd);
        return NULL;
    }

    int pret = phr_parse_request(
        buf, n,
        &method, &method_len,
        &path, &path_len,
        &minor_version,
        headers, &num_headers,
        0
    );

    if (pret <= 0) {
        const char *msg = "HTTP/1.0 400 Bad Request\r\n\r\n";
        send(client_fd, msg, strlen(msg), 0);
        close(client_fd);
        return NULL;
    }

    const char *host = find_header(headers, num_headers, "Host");
    if (!host) {
        const char *msg = "HTTP/1.0 400 Bad Request\r\n\r\nNo Host header";
        send(client_fd, msg, strlen(msg), 0);
        close(client_fd);
        return NULL;
    }

    char host_buf[256];
    snprintf(host_buf, sizeof(host_buf), "%.*s",
             (int)headers[0].value_len, host);

    int server_fd = connect_to_host(host_buf, "80");
    if (server_fd < 0) {
        const char *msg = "HTTP/1.0 502 Bad Gateway\r\n\r\n";
        send(client_fd, msg, strlen(msg), 0);
        close(client_fd);
        return NULL;
    }

    send(server_fd, buf, n, 0);
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

        pthread_t tid;
        if (pthread_create(&tid, NULL, client_thread, arg) != 0) {
            close(client_fd);
            free(arg);
        }
    }
}
