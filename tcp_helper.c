#include "tcp_helper.h"

#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

SSL_CTX *initCtx() {
    OpenSSL_add_all_algorithms(); /* Load cryptos, et.al. */
    SSL_load_error_strings();     /* Bring in and register error messages */
    return SSL_CTX_new(TLS_client_method()); /* Create new context */
}

char *http1_header(const char *http_method, const char *host,
                   const char *route) {
    static const char *template = "%s %s HTTP/1.1\r\nHost: %s\r\n\r\n";
    size_t siz =
        strlen(http_method) + strlen(route) + strlen(host) + strlen(template);
    char *buf = calloc(siz, 1);
    sprintf(buf, template, http_method, route, host);
    return buf;
}

int create_connect(const char *host, const char *service, int family,
                   int sock_type, int protocol, int flag) {
    struct addrinfo hint;
    struct addrinfo *result;
    bzero(&hint, sizeof(hint));
    hint.ai_family = family;
    hint.ai_socktype = sock_type;
    hint.ai_protocol = protocol;
    hint.ai_flags = flag;
    if (getaddrinfo(host, service, &hint, &result)) { // if resolve failed
        return -1;
    }
    for (struct addrinfo *it = result; it != NULL; it = it->ai_next) {
        int sfd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, it->ai_addr, it->ai_addrlen) != -1) {
            freeaddrinfo(result);
            return sfd;
        }
        close(sfd);
    }
    freeaddrinfo(result);
    return -1;
}
