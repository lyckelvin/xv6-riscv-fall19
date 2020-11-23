#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXARGS 10
#define MAXWORD 30

//sh.c�е�getcmd����
int getcmd(char *buf, int nbuf)
{
  //��$�滻��@
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
  //��argv��ÿһ��Ԫ�ض�ָ��args��ÿһ��
  for(int i = 0; i < MAXARGS; i++){
    argv[i] = &args[i][0];
  }
  int i = 0;
  int j = 0;
  for(;cmd[j] != '\n' && cmd[j] != '\0'; j++){
      //����֮ǰ�Ŀո�
      while(strchr(whitespace,cmd[j])){
        j++;
      }
      argv[i++] = cmd + j;
      //������һ���ո�
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
    //������|  pipe
      execPipe(argv,argc);
    }
  }
  for(int i = 1; i < argc; i++){
    //������>,��Ҫִ������ض����ȹر�stdout
    if(!strcmp(argv[i],">")){
      close(1);
      //����ض����һ��������Ӧ�ļ���
      open(argv[i+1],O_CREATE|O_WRONLY);
      argv[i] = 0;
      
    }
    //������<,��Ҫִ�������ض����ȹر�stdin
    if(!strcmp(argv[i],"<")){
      close(0);
      //����ض����һ��������Ӧ�ļ���
      open(argv[i+1],O_RDONLY);
      argv[i] = 0;
      
    }
  }
  exec(argv[0],argv);
}

void execPipe(char* argv[],int argc)
{
  int i;
  // �����ҵ������е�"|",Ȼ���������'\0'
  for(i = 0;i<argc;i++){
    if(!strcmp(argv[i],"|")){
      argv[i]=0;
      break;
    }
  }
  
  int pd[2];
  pipe(pd);
  
  if(fork() == 0){
    //�ر��Լ��ı�׼�����ִ����ߵ�����
    close(1);
    dup(pd[1]);
    close(pd[0]);
    close(pd[1]);
    runcmd(argv, i);
  }else{
    //�ر��Լ��ı�׼���룬ִ���ұߵ�����
    close(0);
    dup(pd[0]);
    close(pd[0]);
    close(pd[1]);
    runcmd(argv+i+1, argc-i-1);
  }
}

//��sh.c �е�main����
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