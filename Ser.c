

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

#define list4each(head,p)     \
            for(p=head->next;p!=head;p=p->next)


void * rev_info(void * arg);
void handler(int arg);
typedef struct list
{
    char name[50];
    int account;
    char ip[18];
    struct list *next;
    struct list *prev;

}list,*p_list;
p_list head=NULL;
pthread_mutex_t lock;

p_list init_list();
p_list create_new_node(char *name,char *ip,int account);
bool add_tolist(p_list head,p_list new);
p_list find_node(p_list head,int account);
bool change_info_from_list(p_list head,int account);
bool delect_node(p_list head,int sockConn);
void *rontinue(void * arg);
bool check_int(char *cmd);

int main()
{
    int socketSer=socket(AF_INET,SOCK_STREAM,0);
    if(socketSer<0)
    {
        perror("socket");
        return 0;
    }

    struct sockaddr_in sockSer,sockCli;
    sockSer.sin_family=AF_INET;
    sockSer.sin_port=htons(SERVER_PORT);
    sockSer.sin_addr.s_addr=inet_addr(SERVER_IP);

    int yes=1;
    if(setsockopt(socketSer,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1)
    {
        perror("");
    }

    socklen_t addrlen=sizeof(struct sockaddr);
    int ret=bind(socketSer,(struct sockaddr *)&sockSer,addrlen);
    if(ret<0)
    {
        perror("bind");
        return 0;
    }

    listen(socketSer,5);
    printf("Server Wait Client Accept .....\n");
    pthread_mutex_init(&lock,NULL);
    head=init_list();
    if(head==NULL)
    {
        printf("malloc error!\n");
    }

    int i=0;
    while(1)
    {
        int sockConn=accept(socketSer,(struct sockaddr *)&sockCli,&addrlen);
        if(sockConn==-1)
        {
            perror("accept:");
            return 0;
        }
        else
        {
            printf("client id:%s\n",inet_ntoa(sockCli.sin_addr));
            printf("client port:%ud\n",ntohs(sockCli.sin_port));
            printf("successful!\n");
        }

        p_list new=create_new_node("xxxxx",inet_ntoa(sockCli.sin_addr),sockConn);
        pthread_mutex_lock(&lock);
        add_tolist(head,new);
        pthread_mutex_unlock(&lock);
        pthread_t tid;
        pthread_create(&tid,NULL,rontinue,&sockConn);
        pthread_detach(tid);
        
    }

    



    close(socketSer);

    return 0;
}

void *rontinue(void * arg)
{
    int sockConn=*(int *)arg;
    printf("sockConn:%d\n",sockConn);
    int select=0;
    char cmd[256];
    while(1)
    {
        printf( "_________菜单_________\n"
                "1.显示所有在线人员列表  \n"
                "2.私聊\n"
                "3.建立群聊\n"
                "______________________\n");
        // scanf("%d",&select);
        bzero(cmd,256);
        read(sockConn,cmd,256);
        printf("cmd :%s\n",cmd);
        if(!check_int(cmd))
        {
            if(!strcmp(cmd,"exit"))
            {   
                pthread_mutex_lock(&lock);
                delect_node(head,sockConn);
                pthread_mutex_unlock(&lock);
                pthread_exit(NULL);
            }
            
        }
        //printf("XXXTTTZZZZ\n");
        if(strlen(cmd)==0)
        {
            pthread_exit(NULL);
        }

        select=atoi(cmd);
       // while('\n'!=getchar());
        //printf("select:%d\n",select);
        switch(select)
        {
            case 1:
            {
                //printf("select:%d\n",select);
                char buf[256]={0};
                p_list p;
                for(p=head->next;p!=head;p=p->next)
                {
                   
                    if(p->account==sockConn)
                    {
                        printf("myname：%s\tip:%s\n",p->name,p->ip);
                        sprintf(buf,"%s,%s",p->name,p->ip);
                        write(sockConn,buf,256);
                    }
                    if(p->account!=sockConn)
                    {
                        printf("name:%s\tip:%s\t\n",p->name,p->ip);
                        sprintf(buf,"%s,%s",p->name,p->ip);
                        write(sockConn,buf,256);
                    }
                        
                }
                write(sockConn,"#end",256);
                break;
            }
            
            case 2://私聊//转发
            {
                char ip[20]={0};
                read(sockConn,ip,20);
                //printf("ip:%s\n",ip);
                
                p_list p;
                char buf[1024]={0};
                list4each(head,p)
                {
                    if(!strcmp(p->ip,ip))
                    {
                        while(1)
                        {
                            bzero(buf,1024);
                            read(sockConn,buf,1024);
                            if(!strcmp(buf,"exit"))
                            {
                                break;
                            }
                            write(p->account,buf,strlen(buf));
                        }
                    }
                }
                break;


            }
            case 3:
            {

            }
        }
        select=0;


    }



}

//初始化链表
p_list init_list()
{
    p_list head=calloc(1,sizeof(list));
    if(head==NULL)
        return NULL;
    head->next=head;
    head->prev=head;
    bzero(head->ip,0);
    bzero(head->name,50);
    return head;

}

//创建新节点
p_list create_new_node(char *name,char *ip,int account)
{
    p_list new=calloc(1,sizeof(list));
    if(new==NULL)
        return NULL;
    new->next=new;
    new->prev=new;
    strncpy(new->ip,ip,strlen(ip));
    new->account=account;
    strncpy(new->name,name,strlen(name));
    return new;
}

//添加
bool add_tolist(p_list head,p_list new)
{
    p_list p=head;
    for(p=head;p->next!=head;p=p->next);

    new->next=p->next;
    p->next->prev=new;

    new->prev=p;
    p->next=new;

    return true;
}

//查找
p_list find_node(p_list head,int account)
{
    p_list p;
    for(p=head->next;p!=head;p=p->next)
    {
        if(p->account==account)
        {
            return p;
        }
    }
    return NULL;
}

//修改
bool change_info_from_list(p_list head,int account)
{
    p_list p=find_node(head,account);
    if(p==NULL)
        return false;
    char name[50]={0};    
    printf("请输入新的昵称！\n");
    scanf("%s",name);   
    bzero(p->name,50);
    strncpy(p->name,name,strlen(name));
    return true;

}

//删除
bool delect_node(p_list head,int sockConn)
{
    p_list p=find_node(head,sockConn);
    if(p==NULL)
    {
        return false;
    }
    p->prev->next=p->next;
    p->next->prev=p->prev;

    p->next=NULL;
    p->prev=NULL;
    free(p);
    p=NULL;
}




bool check_int(char *cmd)
{
    for(int i=0;i<strlen(cmd);i++)
    {
        if(cmd[i]<'0'||cmd[i]>'9')
        {
            return false;
        }
    }

    int select =atoi(cmd);
    if(select<0||select>4)
    {
        return false;
    }
    return true;
}