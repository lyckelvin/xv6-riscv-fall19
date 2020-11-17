#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int p1[2], p2[2];
  pipe(p1);
  pipe(p2);
  char buf[64];

  if (fork()) {
    // Parent
    write(p1[1], "ping", strlen("ping"));
    read(p2[0], buf, 4);
    printf("%d: received %s\n", getpid(), buf);
  } else {
    // Child
    read(p1[0], buf, 4);
    printf("%d: received %s\n", getpid(), buf);
    write(p2[1], "pong", strlen("pong"));
  }

  exit();
}