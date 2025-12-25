#include "pollserver.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_DATA_SIZE 256

char const *psSocketToIPString(void *addr, char *buf, size_t size)
{
  struct sockaddr_storage *sas = addr;
  struct sockaddr_in      *sa4;
  struct sockaddr_in6     *sa6;

  void *src; // Where to pull IP from

  switch (sas->ss_family)
  {
  case AF_INET:
  {
    sa4 = addr;
    src = &(sa4->sin_addr);
    break;
  }
  case AF_INET6:
  {
    sa6 = addr;
    src = &(sa6->sin6_addr);
    break;
  }
  default:
  {
    fprintf(stderr, "SocketToIPString: ss_family not recognized\n");
    return NULL;
  }
  }

  return inet_ntop(sas->ss_family, src, buf, size);
}

int psGetListenerSocket(void)
{
  int             listener_fd;
  int             rv;
  struct addrinfo hints, *ai, *p;

  memset(&hints, 0, sizeof hints);

  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE; // Autofill IP

  rv = getaddrinfo(NULL, PORT, &hints, &ai);
  if (rv != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(EXIT_FAILURE);
  }

  for (p = ai; p != NULL; p = p->ai_next)
  {
    // Socket
    listener_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener_fd < 0)
    {
      continue;
    }

    // Make reusable
    int yes = 1;
    if (setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) != 0)
    {
      continue;
    };

    // Bind
    if (bind(listener_fd, p->ai_addr, p->ai_addrlen) != 0)
    {
      continue;
    }

    break;
  }

  // Handle case where we didn't get bound
  if (p == NULL)
  {
    fprintf(stdout, "could not bind\n");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(ai);

  if (listen(listener_fd, BACKLOG) != 0)
  {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  return listener_fd;
}

void psAddPollFd(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
  // Double pointer because the address of pfds[] might change on realloc

  // Make more room by doubling
  if (*fd_count == *fd_size)
  {
    *fd_size *= 2;
    *pfds = realloc(
        *pfds, sizeof(**pfds) * (*fd_size)); // Size of a single fd (int) times fd size
  }

  (*pfds)[*fd_count].fd      = newfd;
  (*pfds)[*fd_count].events  = POLLIN; // What event to scan for
  (*pfds)[*fd_count].revents = 0;      // Is set when event happens; we check for this

  (*fd_count)++;
}

void psDeletePollFd(struct pollfd pfds[], int i, int *fd_count)
{
  // Overwrite the ith pfd with the one at the end
  // TODO: Does this delete the very last one? Try on paper!

  pfds[i] = pfds[*fd_count - 1];
  (*fd_count)++;
}

void psHandleNewConnection(int            listener_fd,
                           int           *fd_count,
                           int           *fd_size,
                           struct pollfd *pfds[])
{
  struct sockaddr_storage their_addr;
  socklen_t               their_addrlen;
  int                     new_fd;
  char                    their_ip_strp[INET6_ADDRSTRLEN];

  their_addrlen = sizeof their_addr;

  new_fd = accept(listener_fd, (struct sockaddr *)&their_addr, &their_addrlen);
  if (new_fd < 0)
  {
    perror("accept");
  }
  else
  {
    psAddPollFd(pfds, new_fd, fd_count, fd_size);

    printf("New connection from %s on socket %d\n",
           psSocketToIPString(&their_addr, their_ip_strp, their_addrlen),
           new_fd);
  }
}

void psHandleClientData(int listener_fd, int *fd_count, struct pollfd pfds[], int *pfd_i)
{
  char buf[MAX_DATA_SIZE];

  int sender_fd = pfds[*pfd_i].fd; // The sender is us

  int bytes_received = recv(sender_fd, buf, sizeof buf, 0);
  if (bytes_received <= 0)
  {
    if (bytes_received == 0)
    {
      // Connection closed case
      printf("Socket %d hung up\n", sender_fd);
    }
    else
    {
      // Receive error
      perror("recv");
    }

    close(sender_fd);
    psDeletePollFd(pfds, *pfd_i, fd_count);

    // Reexamine index we were at
    (*pfd_i)--;
  }
  else
  {
    // NOTE: %.*s prints .* chars from the given string
    printf("Received data from fd %d: %.*s", sender_fd, bytes_received, buf);

    for (int i = 0; i < *fd_count; ++i)
    {
      int dest_fd = pfds[i].fd;

      // Don't send to listener and ourselves
      if (dest_fd != listener_fd && dest_fd != sender_fd)
      {
        int bytes_sent = send(dest_fd, buf, bytes_received, 0);
        if (bytes_sent < 0)
        {
          perror("send");
        }
      }
    }
  }
}

void psProcessConnections(int            listener_fd,
                          int           *fd_count,
                          int           *fd_size,
                          struct pollfd *pfds[])
{
  for (int i = 0; i < *fd_count; ++i)
  {
    // Check revents is set to POLLIN or POLLHUP

    // We check for:
    // POLLIN: Data is ready to be recv() on socket
    // POLLOUT: Data is ready to be send() on socket without blocking
    // POLLHUP: The remote has closed the connection at this socket

    if ((*pfds)[i].revents & (POLLIN | POLLHUP))
    {
      if ((*pfds)[i].fd == listener_fd)
      {
        // If the listener (us) is woken up, we have a new connection
        psHandleNewConnection(listener_fd, fd_count, fd_size, pfds);
      }
      else
      {
        // If not, then it's just an already connected client bugging us
        psHandleClientData(listener_fd, fd_count, *pfds, &i);
      }
    }
  }
}
