/**
 * @file pollserver.h
 * @brief Defines pollserver functions
 */

#ifndef POLLSERVER_H
#define POLLSERVER_H

#include <poll.h>
#include <stdio.h>

/**
 * @brief The port the server will listen on
 */
#define PORT "3490"

/**
 * @brief Ammount of connections waiting to be accepted
 */
#define BACKLOG 8

/**
 * @brief Converts an address into a printable IP string
 * @param addr The address to convert
 * @param buf The buffer to write the printable IP string to
 * @param size The size of the buffer
 * @return C-style IP string
 */
char const *psSocketToIPString(void *addr, char *buf, size_t size);

/**
 * @brief Create a socket to listen for connections on
 * @return File descriptor to the listener socket
 */
int psGetListenerSocket(void);

/**
 * @brief Adds a new file descriptor to a list of pollfds
 * @param pfds A pointer to the list of pollfds
 * @param newfd The file descriptor to add
 * @param fd_count The amount of file descriptors currently in the list
 * @param fd_size The size of the list of pollfds
 */
void psAddPollFd(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size);

/**
 * @brief Deletes a file descriptor from a list of pollfds
 * @param pfds The list of pollfds
 * @param i The index of the file descriptor to delete
 * @param fd_count The amount of file descriptors currently in the list
 */
void psDeletePollFd(struct pollfd pfds[], int i, int *fd_count);

/**
 * @brief Accepts an incoming connection and adds its fd to a list of pollfds
 * @param listener_fd The listener socket file descriptor
 * @param fd_count The amount of file descriptors currently in the list
 * @param fd_size The size of the list of pollfds
 * @param pfds Pointer to the list of pollfds
 */
void psHandleNewConnection(int            listener_fd,
                           int           *fd_count,
                           int           *fd_size,
                           struct pollfd *pfds[]);

/**
 * @brief Handles receiving client data
 * @param listener_fd The listener socket file descriptor
 * @param fd_count The amount of file descriptors currently in the list
 * @param pfds The list of pollfds
 * @param pfd_i Pointer to the index of the sender client
 */
void psHandleClientData(int listener_fd, int *fd_count, struct pollfd pfds[], int *pfd_i);

/**
 * @brief Handles receiving client data
 * @param listener_fd The listener socket file descriptor
 * @param fd_count The amount of file descriptors currently in the list
 * @param fd_size The size of the list of pollfds
 * @param pfds Pointer to the list of pollfds
 */
void psProcessConnections(int            listener_fd,
                          int           *fd_count,
                          int           *fd_size,
                          struct pollfd *pfds[]);

#endif
