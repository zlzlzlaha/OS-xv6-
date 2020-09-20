#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"


struct userinfo
{
    char valid;
    char  id[17];
    char  password[17];
};


int main(int argc, char *argv[])
{
    int fd;
    int i;
    int pid;
    char password[17];
    char id[17];
    char *argv2[] = {"sh",0};

    struct userinfo usertable[10];
    while(1)
    {
        if((fd = open("userlist",O_RDWR)) < 0 )
            printf(1,"failed to open userlist\n");
        
        
        read(fd,&usertable,sizeof(usertable));
        /*
        for(i = 0 ; i < 10 ; i++)
        {
            if(usertable[i].valid == 1)
            {
                printf(1,"valid: %d id :%s  password :%s \n", usertable[i].valid, usertable[i].id, usertable[i].password);
            }
        }
        */

        while(1)
        {   
            printf(1,"Username: ");
            gets(id,100);
            id[strlen(id)-1] = '\0';
            printf(1,"Password: "); 
            gets(password,100);
            password[strlen(password)-1] = '\0';
      
            for(i = 0 ; i <10 ; i ++)
            {
                if(usertable[i].valid && (strcmp(id,usertable[i].id) ==0 ) && ( strcmp(password,usertable[i].password) ==0))
                    break;
            }
            if(i < 10)
                break;
            printf(1,"wrong login information\n");
        }
        close(fd);

        pid = fork();
        if(pid == 0)
        {
           change_user(id);
           exec("sh",argv2);
           printf(1,"exec sh faield\n");
           exit();
        }
        wait();
    }
    exit();
}    
