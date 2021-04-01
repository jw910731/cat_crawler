#include <stdint.h>
#include <openssl/ssl.h>

#pragma once

/*
 * Create connection socket fd to given filter
 */
int create_connect(const char *host, const char *service, int family,
                   int sock_type, int protocol, int flag);

/*
 * Return HTTP header with corresponding HTTP method
 */
char *http1_header(const char *http_method, const char *host,
                   const char *route);

/*
 * Create New SSL Context
 * @return New SSL context that may be NULL
 */
SSL_CTX *initCtx();