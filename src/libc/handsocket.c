/*============================================================================*
 * Imports                                                                    *
 *============================================================================*/

#include <handsocket.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/*============================================================================*
 * Private Functions                                                          *
 *============================================================================*/

/**
 * @brief Open socket using default values.
 *
 * @return Upon succesfull socket address is returned, otherwise
 * a negative errno.
 */
int open_socket () {

    // 
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        printf("\n ### ERROR: Failed to create socket. errno: %i \n", errno);
    }

    return sockfd;
}

/**
 * @brief Close a socket.
 *
 * @return Upon succesfull zero is returned, otherwise
 * a negative errno.
 */
int close_socket (int sockfd) {
    int ret = close(sockfd);

    if (ret < 0) {
        printf("\n ### ERROR: Failed to close socket. errno: %i \n", errno);
    }

    return ret;
}

/** 
 * @brief Set a timeout for socket finish transmission before
 * close.
 *
 * @return Upon succesfull zero is returned, otherwise
 * a negative errno.
 */
int settimeout_socket(int sockfd) {
    struct linger opt;

    opt.l_onoff = 1;
    opt.l_linger = CONNECTION_TIMEOUT;

    // Set socket timeout. 
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_LINGER,
                &opt, sizeof(opt));

    if (ret < 0) {
        printf("\n ### ERROR: Failed to set timeout in socket. errno: %i \n", errno);
    }

    return ret;
}

/**
 * @brief Bind a address to the socket.
 *
 * @return Upon succesfull zero is returned. otherwise
 * a negative errno.
 */
int setaddr_socket(int sockfd, int port) {

    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the addres to the socket.
    int ret = bind(sockfd, (struct sockaddr *) (&addr), sizeof(struct sockaddr));

    if (ret < 0) {
        printf("\n ### ERROR: Failed to bind the address. errno: %i \n", errno);
    }

    return ret;
}

/**
 * @brief Set the socket to listen and accept connections
 * the lenght of the queue for pending connections is defined
 * by MAX_QUEUE.
 *
 * @return Upon succesfull zero is returned. otherwise
 * a negative errno.
 */
int listen_socket(int sockfd) {

    // Set socket to listen.
    int ret = listen(sockfd, MAX_QUEUE);

    if (ret < 0) {
        printf("\n ### ERROR: Failed to set socket to listen. errno: %i \n", errno);
    }

    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    if (getsockname(sockfd, (struct sockaddr *) &addr, &len) < 0){
        printf("\n ### ERROR: Failed to get socket port. errno: %i \n", errno);
    } else {
        printf("\n Listening on %i\n", addr.sin_port);
    }

    return ret;
}


/**
 * @brief Accepts a connection from the socket and
 * creates a new socket connected to the client.
 *
 * @return Upon succesfull zero is returned. otherwise
 * a negative errno.
 */
int accept_connection (int sockfd) {
    struct sockaddr addr;
    socklen_t length = 0;

    int ret = accept(sockfd, &addr, &length);

    if (ret < 0) {
        printf("\n ### ERROR: Failed to accept a connection. errno: %i \n", errno);
    }

    return ret;
}
