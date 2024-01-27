#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

// NOTE: See `https://bruinsslot.jp/post/simple-http-webserver-in-c/`.

typedef int32_t i32;

typedef uint8_t  u8;
typedef uint32_t u32;

typedef ssize_t isize;

typedef struct sockaddr_in SockAddrIn;
typedef struct in_addr     InAddr;
typedef socklen_t          SockLen;
typedef struct sockaddr    SockAddr;

#define OK    0
#define ERROR 1

#define PRINT_ERRNO()                                                \
    do {                                                             \
        fprintf(stderr, "%s:%s:%d: ", __FILE__, __func__, __LINE__); \
        perror(NULL);                                                \
    } while (0)

#define EXIT_IF_ERRNO(condition) \
    do {                         \
        if (condition) {         \
            PRINT_ERRNO();       \
            _exit(ERROR);        \
        }                        \
    } while (0)

#define PORT 8080

#define RESPONSE                      \
    "HTTP/1.0 200 OK\r\n"             \
    "Server: http_server\r\n"         \
    "Content-type: text/html\r\n\r\n" \
    "<html><h3>Hello, world!</h3></html>\r\n"

#define CAP_BUFFER (1 << 12)
static char BUFFER[CAP_BUFFER];

#define CAP_METHOD  (1 << 6)
#define CAP_URI     (1 << 6)
#define CAP_VERSION (1 << 6)

i32 main(void) {
    i32 host_socket = socket(AF_INET, SOCK_STREAM, 0);
    EXIT_IF_ERRNO(host_socket == -1);
    printf("  [ Socket created successfully ]\n");

    i32 option = 1;
    EXIT_IF_ERRNO(setsockopt(host_socket,
                             SOL_SOCKET,
                             SO_REUSEADDR,
                             &option,
                             sizeof(option)) == -1);

    SockAddrIn host_addr;
    SockLen    host_len = sizeof(host_addr);

    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // host_addr.sin_addr.s_addr = inet_addr("192.168.1.152");

    EXIT_IF_ERRNO(bind(host_socket, (SockAddr*)&host_addr, host_len) != 0);
    printf("  [ Socket successfully bound to address ]\n");

    {
        char buffer_address[NI_MAXHOST];
        char buffer_port[NI_MAXSERV];
        if (getnameinfo((SockAddr*)&host_addr,
                        host_len,
                        buffer_address,
                        NI_MAXHOST,
                        buffer_port,
                        NI_MAXSERV,
                        NI_NUMERICHOST | NI_NUMERICSERV) == 0)
        {
            printf("  [ address: %s:%s ]\n", buffer_address, buffer_port);
        }
    }

    EXIT_IF_ERRNO(listen(host_socket, SOMAXCONN) != 0);
    printf("  [ Server listening for connections ]\n");

    for (u32 i = 0; i < 100; ++i) {
        SockAddrIn client_addr;
        SockLen    client_len = sizeof(client_addr);

        i32 client_socket =
            accept(host_socket, (SockAddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            PRINT_ERRNO();
            continue;
        }
        printf("  [ Connection accepted ]\n");

        isize read_len = read(client_socket, BUFFER, CAP_BUFFER);
        if (read_len < 0) {
            PRINT_ERRNO();
            continue;
        }
        printf("\n%.*s", (i32)read_len, BUFFER);

        {
            char method[CAP_METHOD];
            char uri[CAP_URI];
            char version[CAP_VERSION];

            sscanf(BUFFER, "%s %s %s", method, uri, version);

            printf("  [ "
                   "socket: %d, "
                   "address: %hhu.%hhu.%hhu.%hhu:%u, "
                   "method: %s, "
                   "version: %s, "
                   "uri: %s "
                   "]\n",
                   client_socket,
                   (u8)(client_addr.sin_addr.s_addr & 0xFFu),
                   (u8)((client_addr.sin_addr.s_addr & 0xFF00u) >> 8u),
                   (u8)((client_addr.sin_addr.s_addr & 0xFF0000u) >> 16u),
                   (u8)((client_addr.sin_addr.s_addr & 0xFF000000u) >> 24u),
                   ntohs(client_addr.sin_port),
                   method,
                   version,
                   uri);
        }

        if (write(client_socket, RESPONSE, sizeof(RESPONSE) - 1) == -1) {
            PRINT_ERRNO();
            continue;
        }

        close(client_socket);
    }

    close(host_socket);

    return OK;
}
