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
    int ret;
    int i;
    char password[17];
    char id[17];

    struct userinfo usertable[10];
    
    if((fd = open("userlist",O_RDWR)) < 0 )
        printf(1,"failed to open userlist\n");

    //Debug
    ret =read(fd,&usertable,sizeof(usertable));
    printf(1,"ret %d\n",ret);

    for(i = 0 ; i < 10 ; i++)
    {
       if(usertable[i].valid == 1)
       {
         printf(1,"valid: %d id :%s  password :%s \n", usertable[i].valid, usertable[i].id, usertable[i].password);
       }
    }

    while(1)
    {
      printf(1,"input id : ");
      gets(id,100);
      id[strlen(id)-1] = '\0';
      printf(1,"input password : "); 
      gets(password,100);
      password[strlen(password)-1] = '\0';
      
      for(i = 0 ; i <10 ; i ++)
      {
        if(usertable[i].valid && (strcmp(id,usertable[i].id) ==0 ) && ( strcmp(password,usertable[i].password) ==0))
            break;
      }
      if(i < 10)
         break;
      printf(1,"wrong id/password\n");
    }

    close(fd);


    exit();
}    
