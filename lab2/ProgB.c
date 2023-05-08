#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <wait.h>

#define DEBUG_MODE 0

char **GetArgsFromStr(char *argsStr) {
  const char *DELIMS = " \"";

  char **argsArr = NULL;
  char *arg = strtok(argsStr, DELIMS);

  int i = 0;
  while (arg != NULL) {
    argsArr = reallocarray(argsArr, i + 1, sizeof(char *));
    argsArr[i] = arg;
    arg = strtok(NULL, DELIMS);
    i++;
  }

  argsArr = reallocarray(argsArr, i + 1, sizeof(char *));
  argsArr[i] = arg;

  if (DEBUG_MODE) {
    printf("{");
    for (int j = 0; j < i; j++) {
      printf("\"%s\"", argsArr[j]);
      if (i - 1 != j)
      {
        printf(", ");
      }
    }
    printf("}\n");
  }
  return argsArr;
}

int main() {

  char cmdTest[] = "cat testFile.txt | sort";

  char *cmd;
  if (DEBUG_MODE) {
    cmd = cmdTest;
  } else {
    cmd = readline("ProgB is ready: ");
  }

  const char *DELIMS = "|";
  char *args1 = strtok(cmd, DELIMS);

  if (!DEBUG_MODE) {
    kill(getppid(), SIGUSR1);
  }
  while(1);

  int pipe1[2] = {-1, -1};
  int pipe2[2];

  while (args1 != NULL) {
    if (DEBUG_MODE) {
      printf("Args1: %s\n", args1);
    }

    char *args2 = strtok(NULL, DELIMS);
    if (args2 != NULL) { // check if we need another pipe
      pipe(pipe2);
    }
    int childPID = fork();

    if (childPID == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
    }

    if (childPID == 0) { // check if current process is a child process

      if (DEBUG_MODE) {
        printf("ProgB's fork PID is %d\n", getpid());
      }

      char **argsArr = GetArgsFromStr(args1);

      if (pipe1[0] != -1) { // if previous pipe exists
        close(pipe1[1]);
        dup2(pipe1[0], 0);
      }

      if (args2 != NULL) { // check if next pipe exists
        close(pipe2[0]);
        dup2(pipe2[1], 1);
      }

      prctl(PR_SET_PDEATHSIG, SIGTERM);
      
      if (execvp(argsArr[0], argsArr) == -1) {
        perror("execvp");
        kill(getppid(), SIGTERM);
        exit(EXIT_FAILURE);
      }
    }

    close(pipe1[0]);
    close(pipe1[1]);
    args1 = args2;
    pipe1[0] = pipe2[0];
    pipe1[1] = pipe2[1];

    int wstatus;
    wait(&wstatus);

    if (DEBUG_MODE)
    {
      printf("Child %d wait status: %d\n", childPID, wstatus);
    }

    if (wstatus != 0)
    {
      exit(EXIT_FAILURE);
    }
  }

  if(!DEBUG_MODE) {
    free(cmd);
  }


  return 0;
}
