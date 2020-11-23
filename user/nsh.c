#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXARGS 10
#define MAXWORD 30

//sh.c中的getcmd函数
int getcmd(char *buf, int nbuf)
{
  //将$替换成@
  fprintf(2, "@ ");
  memset(buf, 0, nbuf);
  gets(buf, nbuf);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

void execPipe(char* argv[],int argc);

char whitespace[] = " \t\r\n\v";
char args[MAXARGS][MAXWORD];

void setargs(char *cmd, char* argv[], int* argc)
{
  //让argv的每一个元素都指向args的每一行
  for(int i = 0; i < MAXARGS; i++){
    argv[i] = &args[i][0];
  }
  int i = 0;
  int j = 0;
  for(;cmd[j] != '\n' && cmd[j] != '\0'; j++){
      //跳过之前的空格
      while(strchr(whitespace,cmd[j])){
        j++;
      }
      argv[i++] = cmd + j;
      //跳过下一个空格
      while(strchr(whitespace,cmd[j]) == 0){
        j++;
      }
      cmd[j] ='\0';
  }
  argv[i] = 0;
  *argc = i;
}

void runcmd(char* argv[],int argc)
{
  for(int i = 1; i < argc; i++){
    if(!strcmp(argv[i],"|")){
    //遇到了|  pipe
      execPipe(argv,argc);
    }
  }
  for(int i = 1; i < argc; i++){
    //遇到了>,需要执行输出重定向，先关闭stdout
    if(!strcmp(argv[i],">")){
      close(1);
      //输出重定向后到一参数到对应文件中
      open(argv[i+1],O_CREATE|O_WRONLY);
      argv[i] = 0;
      
    }
    //遇到了<,需要执行输入重定向，先关闭stdin
    if(!strcmp(argv[i],"<")){
      close(0);
      //输出重定向后到一参数到对应文件中
      open(argv[i+1],O_RDONLY);
      argv[i] = 0;
      
    }
  }
  exec(argv[0],argv);
}

void execPipe(char* argv[],int argc)
{
  int i;
  // 首先找到命令中的"|",然后把他换成'\0'
  for(i = 0;i<argc;i++){
    if(!strcmp(argv[i],"|")){
      argv[i]=0;
      break;
    }
  }
  
  int pd[2];
  pipe(pd);
  
  if(fork() == 0){
    //关闭自己的标准输出，执行左边的命令
    close(1);
    dup(pd[1]);
    close(pd[0]);
    close(pd[1]);
    runcmd(argv, i);
  }else{
    //关闭自己的标准输入，执行右边的命令
    close(0);
    dup(pd[0]);
    close(pd[0]);
    close(pd[1]);
    runcmd(argv+i+1, argc-i-1);
  }
}

//简化sh.c 中的main函数
int main(void)
{
  static char buf[100];

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    
    if(fork() == 0){
    
      char* argv[MAXARGS];
      int argc = -1;
      
      setargs(buf,argv,&argc);
      runcmd(argv,argc);
    }
    wait(0);
  }
  exit(0);
}