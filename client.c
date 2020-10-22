

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <dirent.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>


#define SERVER_IP "192.168.71.70"
#define SERVER_PORT 60002
#define QUEUE_SIZE 5
int socketcli;

void * rev_info(void * arg);
void handler(int arg);
pthread_t tid;
int flag=1;
void handler(int arg)
{
    write(socketcli,"exit",256);
    exit(0);
}

int main()
{
    socketcli=socket(AF_INET,SOCK_STREAM,0);
    if(socketcli<0)
    {
        perror("socket");
        return 0;
    }

    struct sockaddr_in sockSer;
    sockSer.sin_family=AF_INET;
    sockSer.sin_port=htons(SERVER_PORT);
    sockSer.sin_addr.s_addr=inet_addr(SERVER_IP);

    socklen_t len=sizeof(struct sockaddr);
    printf("connect .....\n");
    int ret=connect(socketcli,(struct sockaddr *)&sockSer,len);
    if(ret==-1)
    {
        perror("connect:");
        return 0;
    }
    else
    {
        printf("successful!\n");
    }

    signal(2,handler);
    pthread_t tid;
    pthread_create(&tid,NULL,rev_info,&socketcli);
    int select=0;
    while(1)
    {
        flag=1;
        printf( "_________菜单_________\n"
                "1.显示所有在线人员列表  \n"
                "2.私聊\n"
                "3.建立群聊\n"
                "4.退出\n"
                "______________________\n");
        scanf("%d",&select);
        char cmd[256]={0};
        while('\n'!=getchar());

        //printf("cmd:%s\n",cmd);
        switch(select)
        {
            case 1:
            {
                flag=1;
                int num=0;
                sprintf(cmd,"%d",select);
                write(socketcli,cmd,256);
                //printf("cmd:%s\n",cmd);
                char buf[256]={0};
                printf("当前好友列表：\n");                
                printf("------------------------------\n");
                while(1)
                {
                    char name[50]={0};
                    char ip[20]={0};
                    bzero(buf,256);
                    read(socketcli,buf,256);
                    if(!strcmp(buf,"#end"))
                    {
                        break;
                    }
                    strcpy(name,strtok(buf,","));
                    strcpy(ip,strtok(NULL,","));

                    printf("name:%s\t ip:%s\n",name,ip);
                    num++;
                }
                printf("当前在线人数：%d\n",num);
                printf("------------------------------\n");
                select=0;
                break;
            }
            case 2:
            {
                sprintf(cmd,"%d",select);
                write(socketcli,cmd,256);
                char ip[20]={0};
                printf("请输入正确的ip地址：(例如192.168.71.12)\n");
                scanf("%s",ip);
                write(socketcli,ip,20);
                char buf[1024]={0};
                flag=0;
                while(1)
                {
                    bzero(buf,1024);
                    printf("请输入内容:\n");
                    fgets(buf,1024,stdin);
                    write(socketcli,buf,strlen(buf)-1);
                    if(!strcmp(buf,"exit"))
                    {
                        printf("退出聊天状态...\n");
                        break;
                    }

                }
                flag=1;
                break;
            }
            case 3:
            {
                
            }
            case 4:
            {
                
                write(socketcli,"exit",256);
                sleep(5);
                close(socketcli);
                exit(0); 
            }
            
        }





    }


    close(socketcli);
    return 0;
}
void * rev_info(void * arg)
{
    int socketcli=*(int *)arg;
    char revbuf[1024]={0};
    while(1)
    {
        
        if(flag==1)
            continue;
        bzero(revbuf,1024);
        read(socketcli,revbuf,1024);
        if(strlen(revbuf)>0)
            printf("from :%s\n",revbuf);
    }
}