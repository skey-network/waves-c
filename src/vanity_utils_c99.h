#include <sys/time.h>

struct timeval;

void timersub(struct timeval *a, struct timeval *b, struct timeval *result)
{
  (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;
  (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;
  if ((result)->tv_usec < 0)
  {
    --(result)->tv_sec;
    (result)->tv_usec += 1000000;
  }

  // return (stop->tv_sec + stop->tv_usec / 1000000.0) - (start->tv_sec + start->tv_usec / 1000000.0)
}

void usleep(int useconds)
{
  sleep(0.000001 * useconds);
}