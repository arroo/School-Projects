/*
 * dummy exploit program
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TARGET "/usr/local/bin/submit"

int main(void)
{
  char *args[4];
  char *env[1];

  // one way to invoke submit
  system("/usr/local/bin/submit file.txt message");

  // another way
  args[0] = TARGET; args[1] = "file.txt"; 
  args[2] = "message"; args[3] = NULL;

  env[0] = NULL;

  if (execve(TARGET, args, env) < 0)
    fprintf(stderr, "execve failed.\n");

  exit(0);
}
