#include "pollserver.h"
#include <stdlib.h>

int main(void)
{
  int listener_fd;
  int fd_size  = 5; // Start off with room for 5 conns; we will realloc as necessary
  int fd_count = 0;

  struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

  listener_fd = psGetListenerSocket();
  if (listener_fd < 0)
  {
    fprintf(stderr, "Failed to get listener socket\n");
    exit(EXIT_FAILURE);
  }

  // Add new listener to fds
  pfds[0].fd     = listener_fd;
  pfds[0].events = POLLIN; // Wake on POLLIN
  fd_count       = 1;

  puts("Waiting for connections...");

  for (;;) // NOTE: For loop with 3 expressions ommitted = loop forever
  {
    // Thread sleeps here until event is ready
    int poll_count = poll(pfds, fd_count, -1); // Timeout of 0 will cause immediate exit

    // Will continue here when event is picked up
    if (poll_count < 0)
    {
      perror("poll");
      exit(EXIT_FAILURE);
    }

    psProcessConnections(listener_fd, &fd_count, &fd_size, &pfds);
  }
}
