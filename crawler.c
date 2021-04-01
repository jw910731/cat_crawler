#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#include "tcp_helper.h"

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

int exit_stat = EXIT_SUCCESS;
const char *target = "cataas.com";

char *strtoken(char *s, const char *delim) {
    const size_t ds = strlen(delim);
    for (char *it = s; *s; ++it) {
        if (strncmp(it, delim, ds) == 0) {
            memset(it, 0x00, ds);
            return it + ds;
        }
    }
    return NULL;
}

int main() {
    SSL_library_init();
    // setup SSL Context
    SSL_CTX *ctx = initCtx();
    if (ctx == NULL) {
        fprintf(stderr, "initCtx():");
        ERR_print_errors_fp(stderr);
        return EXIT_FAILURE;
    }

    // prepare socket fd
    int sock_fd = create_connect(target, "https", AF_INET, 0, 0, 0);
    if (sock_fd == -1) {
        fputs("Failed to resolve host and establish connection.", stderr);
        return EXIT_FAILURE;
    }

    // set sock timeout
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, (void *)&tv,
               sizeof(struct timeval));
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (void *)&tv,
               sizeof(struct timeval));

    // prepare SSL tunnel
    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock_fd);
    SSL_set_tlsext_host_name(ssl, target);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    if (SSL_connect(ssl) == -1) {
        fprintf(stderr, "SSL_connect():");
        ERR_print_errors_fp(stderr);
        exit_stat = EXIT_FAILURE;
        goto cleanup;
    }

    // send request
    char *req = http1_header("GET", target, "/cat");
    size_t siz = strlen(req);
    int ret = SSL_write(ssl, req, siz);
    if (ret <= 0) {
        if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_READ) {
            fputs("Connection Timed out\n", stderr);
        } else {
            fprintf(stderr, "SSL_write():");
            ERR_print_errors_fp(stderr);
        }
        goto cleanup;
    }
    free(req);
    static char buf[0x1000];
    ret = SSL_read(ssl, buf, 0x1000);
    if (ret <= 0) {
        if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_READ) {
            fputs("Connection Timed out\n", stderr);
        } else {
            fprintf(stderr, "SSL_read():");
            ERR_print_errors_fp(stderr);
        }
        goto cleanup;
    }
    int64_t content_size = strtoll(
        strstr(buf, "Content-Length: ") + strlen("Content-Length: "), NULL, 10);
    // cutoff http header
    char *tmp = strtoken(buf, "\r\n\r\n");
    write(fileno(stdout), tmp, ret - (tmp - buf));
    content_size -= ret - (tmp - buf);
    while (content_size > 0) {
        ret = SSL_read(ssl, buf, 0x1000);
        if (ret <= 0) {
            if (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_READ) {
                fputs("Connection Timed out\n", stderr);
            } else {
                fprintf(stderr, "SSL_read():");
                ERR_print_errors_fp(stderr);
            }
            goto cleanup;
        }
        write(fileno(stdout), buf, ret);
        content_size -= ret;
    }
cleanup:
    shutdown(sock_fd, SHUT_RDWR);
    SSL_CTX_free(ctx);
    return exit_stat;
}
