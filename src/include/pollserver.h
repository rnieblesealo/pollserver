#ifndef POLLSERVER_H
#define POLLSERVER_H

#include <poll.h>
#include <stdio.h>

#define PORT "3490"
#define BACKLOG 8

char const *psSocketToIPString(void *addr, char *buf, size_t size);
int         psGetListenerSocket(void);
void        psAddPollFd(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size);
void        psDeletePollFd(struct pollfd pfds[], int i, int *fd_count);
void        psHandleNewConnection(int            listener_fd,
                                  int           *fd_count,
                                  int           *fd_size,
                                  struct pollfd *pfds[]);

#endif
