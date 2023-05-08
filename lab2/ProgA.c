#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <time.h>
#include <unistd.h>

#define WAIT_SECONDS 3
#define PROG_B_NAME "./ProgB"

pid_t g_childPID = 0;

void SigHandler(int signum) {
  int wstatus;
  clock_t clockStart = clock();
  while ((float)(clock() - clockStart ) / CLOCKS_PER_SEC < WAIT_SECONDS) {
    if (waitpid(g_childPID, &wstatus, WNOHANG) == g_childPID) {
      exit(wstatus);
    }
  }
  if (waitpid(g_childPID, &wstatus, WNOHANG) == 0) {
    kill(g_childPID, SIGTERM);
  }
}

int main() {
  signal(SIGUSR1, SigHandler);
  g_childPID = fork();
  if (g_childPID == -1)
  {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  if (g_childPID == 0) {
    char *args[] = {PROG_B_NAME, NULL};
    execv(args[0], args);
  }

  int wstatus;
  wait(&wstatus);

  return 0;
}
