
#include "../throw/throw.h"


int
main (void) {
  error_t eperm = error_new(EPERM, "EPERM");
  error_t enoent = error_new(ENOENT, "ENOENT");
  error_t esrch = error_new(ESRCH, "ESRCH");
  error_t eintr = error_new(EINTR, "EINTR");

  throw(&eperm, "Operation not permitted");
  throw(&enoent, "No such file or directory");
  throw(&esrch, "No such process");
  throw(&eintr, "Interrupted system call");

  return 0;
}
