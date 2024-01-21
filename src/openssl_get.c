#include <netdb.h>
#include <stdio.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

typedef int32_t  i32;
typedef uint64_t u64;
typedef size_t   usize;

typedef struct addrinfo AddrInfo;
typedef SSL             SslConnection;
typedef SSL_CTX         SslContext;

#define OK    0
#define ERROR 1

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

#define EXIT()                                              \
    do {                                                    \
        printf("%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        _exit(ERROR);                                       \
    } while (FALSE)

#define EXIT_WITH(x)                                                \
    do {                                                            \
        printf("%s:%s:%d `%s`\n", __FILE__, __func__, __LINE__, x); \
        _exit(ERROR);                                               \
    } while (FALSE)

#define EXIT_IF(condition)         \
    do {                           \
        if (condition) {           \
            EXIT_WITH(#condition); \
        }                          \
    } while (FALSE)

#define HOST    "example.com"
#define PORT    "443"
#define REQUEST ("GET / HTTP/1.0\r\nHost: " HOST "\r\n\r\n")

#define CAP_BUFFER (1 << 10)

static char BUFFER[CAP_BUFFER];

i32 main(void) {
    AddrInfo* addrs;
    {
        AddrInfo hints = {0};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        EXIT_IF(getaddrinfo(HOST, PORT, &hints, &addrs));
    }
    i32 descriptor = -1;
    for (AddrInfo* addr = addrs; addr; addr = addr->ai_next) {
        descriptor = -1;
        descriptor =
            socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (descriptor == -1) {
            break;
        }
        if (connect(descriptor, addr->ai_addr, addr->ai_addrlen) == 0) {
            break;
        }
        close(descriptor);
    }
    freeaddrinfo(addrs);
    EXIT_IF(descriptor == -1);

    SslContext* context = SSL_CTX_new(TLS_client_method());
    EXIT_IF(context == NULL);

    SslConnection* connection = SSL_new(context);
    SSL_set_fd(connection, descriptor);

    if (SSL_connect(connection) <= 0) {
        u64 error = ERR_get_error();
        printf("%s\n", ERR_error_string(error, NULL));
        EXIT();
    }

    SSL_write(connection, REQUEST, (sizeof REQUEST) - 1);

    for (;;) {
        const i32 n = SSL_read(connection, BUFFER, CAP_BUFFER);
        EXIT_IF(n < 0);
        if (n == 0) {
            break;
        }
        fwrite(BUFFER, 1, (usize)n, stdout);
    }

    SSL_set_shutdown(connection, SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
    SSL_shutdown(connection);
    SSL_free(connection);
    SSL_CTX_free(context);

    close(descriptor);

    return OK;
}
