/**
 *  TCP transport implementationn
 *
 *  All rights reserved
 *  Tiny Labs Inc
 *  2019
 */
#include "TCPTransport.h"

#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define DEFAULT_PORT  7878

TCPTransport::TCPTransport (void)
  : Transport ()
{

}

TCPTransport::~TCPTransport ()
{

}

int TCPTransport::Open (char *id)
{
  uint16_t port = DEFAULT_PORT;
  char *pstr;
  int i = 0;
  
  // Only ipv4 will have . separator
  if (strchr (id, '.'))
    ipv6 = false;
  else
    ipv6 = true;  

  // Convert string to ip
  if (ipv6) {

    // Count ':' occurences
    while (*id++)
      if (*id == ':')
        i++;

    // If there are 6 strip last
    if ((i == 3) || (i == 6)) {
      pstr = strrchr (id, ':');
      if (pstr) {
        *pstr = '\0';
        port = strtoul (pstr+1, NULL, 0);
      }
    }
    
    // Save address
    inet_pton (AF_INET6, id, &a6.sin6_addr);

    // Create socket
    sockfd = socket (AF_INET6, SOCK_STREAM, 0);
    if (sockfd  < 0)
      return -1;
    
    // Setup type and port
    a6.sin6_family = AF_INET6;
    a6.sin6_port = htons (port);

    // Connect to server
    if (connect (sockfd, (const sockaddr *)&a6, sizeof (a6)) != 0)
      return -1;
  }
  else {
    // Split off port
    pstr = strrchr (id, ':');
    if (pstr) {
      *pstr = '\0';
      port = strtoul (pstr+1, NULL, 0);
    }
    
    // Save address
    inet_pton (AF_INET, id, &a4.sin_addr);

    // Create socket
    sockfd = socket (AF_INET, SOCK_STREAM, 0);
    if (sockfd  < 0)
      return -1;
    
    // Setup type and port
    a4.sin_family = AF_INET;
    a4.sin_port = htons (port);

    // Connect to server
    if (connect (sockfd, (const sockaddr *)&a4, sizeof (a4)) != 0)
      return -1;
  }

  // Flush the socket
  Flush ();
  
  // Success
  return 0;
}

void TCPTransport::Close (void)
{
  // Shutdown socket
  shutdown (sockfd, SHUT_RDWR);
  
  // Close socket
  close (sockfd);
}

void TCPTransport::Flush (void)
{
  int rv, flags;
  uint8_t buf[1];

  // Flush any buffers
  fsync (sockfd);

  // Set as non-blocking
  flags = fcntl (sockfd, F_GETFL, 0);
  fcntl (sockfd, F_SETFL, flags | O_NONBLOCK);

  do {
    rv = read (sockfd, buf, 1);
  } while (rv > 0);

  // Change back to blocking
  fcntl (sockfd, F_SETFL, flags);
}

int TCPTransport::Read (char *buf, int len)
{
  int rv;
  int bread = 0;

  // Grab lock - the entire read must be atomic
  pthread_mutex_lock (&rlock);
  
  while (bread < len) {
    // Read from socket
    rv = read (sockfd, &buf[bread], len - bread);
    if (rv <= 0) {
      pthread_mutex_unlock (&rlock);
      return (rv == -1) ? DEVICE_NOTAVAIL : rv;
    }
    
    // Increment count
    bread += rv;
  }

  // Release lock
  pthread_mutex_unlock (&rlock);
  return bread;
}

int TCPTransport::Write (const char *buf, int len)
{
  int rv;
  int written = 0;

  // Grab lock - the entire read must be atomic
  pthread_mutex_lock (&wlock);
  
  while (written < len) {

    // Read from socket
    rv = write (sockfd, &buf[written], len - written);
    if (rv <= 0) {
      pthread_mutex_unlock (&wlock);
      return (rv == -1) ? DEVICE_NOTAVAIL : rv;
    }
    
    // Increment count
    written += rv;
  }

  // Release lock
  pthread_mutex_unlock (&wlock);
  return written;
}

