/*============================================================================*
 * Imports                                                                    *
 *============================================================================*/

/* System library */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/ip.h>
#include <string.h>

/* Program library */
#include <handlesocket.h>
#include <handleaccept.h>
#include <server.h>
#include <msg.h>

/*============================================================================*
 * Struct                                                                     *
 *============================================================================*/

struct {
    int id;
    int fd;
} client_a[20];

/*============================================================================*
 * Sockets Functions                                                          *
 *============================================================================*/

/**
 * @brief Accepts a connection from the socket and
 * creates a new socket connected to the client.
 *
 * @return Upon succesfull zero is returned. otherwise
 * a negative errno.
 */
int accept_connection(int sockfd)
{
    struct sockaddr addr;
    socklen_t length = 0;

    int ret = accept(sockfd, &addr, &length);

    if (ret < 0) {
        printf("\n ### ERROR: Failed to accept a connection. errno: %i \n", errno);
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
int settimeout_socket(int sockfd)
{
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
int setaddr_socket(int sockfd, int port)
{

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
int listen_socket(int sockfd)
{

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
 * @brief Get value from cli arguments.
 *
 * @return Upon succesfull zero is returned. otherwise
 * a negative error.
 */
int get_port(int argc, char *argv[], int *port)
{
    // Check numbers of args.
    if (argc != 2) {
        printf("\n ### ERROR: Missing Arguments.\n");
        HELPSERVER(argv[0]);
        return -1;
    }

    // Get port and check if is valid.
    *port = atoi(argv[1]);
    if (*port < 1024 || *port > 65536) {
        printf("\n ### ERROR: PORT is out of limit.\n");
        HELPSERVER(argv[0]);
        return -1;
    }
    return 0;
}

/*============================================================================*
 * Service Menssage Functions                                                   *
 *============================================================================*/

/**
 * @brief Service OI menssages.
 *
 * @return Upon succesfull zero is returned. otherwise
 * a negative error.
 */
int service_oi(struct msg_t msg, int sockfd) {

    int id = msg.orig_uid;

    // Reader client.
    if (id > 0 && id < 1000) {
        // Add new reader
        for (int i = 0; i < 10; i++) {
            if (client_a[i].id == -1) {
                client_a[i].id = id;
                client_a[i].fd = sockfd;
                return 0;
            }
        }
        return -1;
    }

    // Sender client.
    if (id > 1000 && id < 1999) {
        // Add new sender
        for (int i = 10; i < 20; i++) {
            if (client_a[i].id == -1) {
                client_a[i].id = id;
                client_a[i].fd = sockfd;
                return 0;
            }
        }
        return -1;
    }

    return -1;
}

/**
 * @brief Service TCHAU menssages.
 *
 * @return Upon succesfull zero is returned. otherwise
 * a negative error.
 */
int service_tchau(struct msg_t msg) {

    int id = msg.orig_uid;

    // Reader client.
    if (id > 0 && id < 1000) {
        // Remove reader.
        for (int i = 0; i < 10; i++) {
            if (client_a[i].id == id) {
                client_a[i].id = -1;
                return 0;
            }
        }
        return -1;
    }

    // Sender client.
    if (id > 1000 && id < 1999) {
        // Remove senders.
        for (int i = 10; i < 20; i++) {
            if (client_a[i].id == id) {
                client_a[i].id = -1;
                return 0;
            }
        }
        return -1;
    }

    return -1;
}

/**
 * @brief Service MSG menssages.
 *
 * @return Upon succesfull zero is returned. otherwise
 * a negative error.
 */
int service_msg(struct msg_t msg) {

    int orig = msg.orig_uid;

    /* Check if client is registered. */
    int aux = -1;
    for (int i = 10; i < 20; i++) {
        if (client_a[i].id == orig) {
            aux = 0;
            break;
        }
    };
    if (aux == -1) {
        return -1;
    }

    int dest = msg.dest_uid;

    // Send to only a dest.
    if (dest >= 0 && dest < 1999) {
        /* Get dest socket*/
        int dest_fd;
        aux = -1;
        for (int i = 0; i < 10; i++) {
            if (client_a[i].id == msg.dest_uid) {
                dest_fd = client_a[i].fd;
                aux = 0;
                break;
            }
        };
        if (aux == -1) {
            return -1;
        }

        int ret = send_msg(dest_fd, msg);
        if (ret < 0) {
            return ret;
        }

        return 0;
    }

    // Send the menssage for all client connected.
    if (dest == 0) {
        for (int i = 0; i < 10; i++) {
            if (client_a[i].id > 0) {
                int ret = send_msg(client_a[i].fd, msg);
                if (ret < 0) {
                    return ret;
                }
            };
        }
        return 0;
    }

    return -1;
}

/*============================================================================*
 * Main Function                                                              *
 *============================================================================*/

int main(int argc, char *argv[])
{
    int ret = 0;
    int port = 0;
    struct msg_t msg;

    fd_set active_fd_set;
    fd_set read_fd_set;

    // Get port from args.
    ret = get_port(argc, argv, &port);
    if (ret < 0) {
        return -1;
    }

    // Open a socket.
    int sockfd = open_socket();
    if (sockfd < 0) {
        return -1;
    }

    printf("Socket = %i \n", sockfd);

    // Naming the socket.
    ret = setaddr_socket(sockfd, port);
    if (ret < 0) {
        return -1;
    }

    // Set a timeout before close a busy socket.
    ret = settimeout_socket(sockfd);
    if (ret < 0) {
        return -1;
    }

    // Set listen to the socket.
    ret = listen_socket(sockfd);
    if (ret < 0) {
        return -1;
    }

    FD_ZERO (&active_fd_set);
    FD_SET (sockfd, &active_fd_set);

    while (1) {
        read_fd_set = active_fd_set;

        // Wait for a input from active sockets.
        if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            printf("\n ### ERROR: Select failed. errno = %i\n", errno);
            return -1;
        }

        // Service sockets with arrive data.
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &read_fd_set)) {
                // Connection request.
                if (i == sockfd) {
                    int newsock = accept_connection(sockfd);
                    if (newsock < 0) {
                        return -1;
                    }
                    // Add new socket in active fd set.
                    FD_SET(newsock, &active_fd_set);
                } else {
                    ret = receive_message(i, &msg);
                    if (ret < 0) {
                        return -1;
                    }
                    switch (msg.type) {
                        case OI:
                            service_oi(msg, i);
                            break;
                        case MSG:
                            service_msg(msg);
                            break;
                        case TCHAU:
                            service_tchau(msg);
                            break;
                        default:
                            printf("\n ### Connection Error: Invalid menssage type.\n");
                            FD_CLR(i, &active_fd_set);
                            ret = close_socket(i);
                            if (ret < 0) {
                                return -1;
                            }
                    }
                }
            }
        }
    }

    ret = close_socket(sockfd);
    if (ret < 0) {
        return -1;
    }

    return 0;
}
