#include "types.h"
#include "stat.h"
#include "user.h"

void strcpyn(char * dest, char *origin,int n);
int find_space(char *s1);


int main(int argc, char *argv[])
{
  int index,index2;
//  int ret;
  char *argv2[10] = {"", 0};
  char command[101];
  char tmp[101] ;
  char path[101];
  int pid, limit,stacksize;
  pid =0;
  limit =0;
  index =0;
  index2 = 0;
  tmp[0] = 1;
  stacksize =0;

  //get pmanager admin in first
  getadmin("2016025687");
  printf(1,"[Process Manager]\n");
  printf(1,"\n");
  while(1)
  {
    printf(1,"> ");
    gets(command,100);
    index = find_space(command);
    strcpyn(tmp,command,index); // copy string before enter, ' ' , '\0'

    // list cmd
    if(strcmp("list",tmp)==0)
    {
        list();
        printf(1,"\n");
    }

    // kill cmd
    else if(strcmp("kill",tmp)==0)
    {
        // get pid from input
        index2 = find_space(command+index+1); // get number information until meet ' ', '\0', enter
        strcpyn(tmp,command+index+1,index2);

        //when first character is minus 
        if(tmp[0] == '-') 
            pid = -1* atoi(tmp+1); //make minus
        else
            pid = atoi(tmp);

        if(kill(pid) == 0)
           printf(1,"kill success pid %d\n",pid);
        else
           printf(1,"failed to kill pid %d\n",pid);

        printf(1,"\n");
    }

    // execute cmd
    else if(strcmp("execute",tmp)==0)
    { 
        //get path from input
        index = index +1;
        index2 = find_space(command+index);
        strcpyn(path,command+index,index2);
       
        argv2[0] = path;

        //get stacksize from input
        index = index + index2+1;
        index2 = find_space(command+index);
        strcpyn(tmp,command+index,index2);

        
        //when first character is minus
        if(tmp[0] == '-')
            stacksize = -1* atoi(tmp+1);
        else
            stacksize = atoi(tmp);

        printf(1,"\n");

        if(fork() ==0)
        {
            //when failed in exec
           if(exec2(argv2[0],argv2,stacksize) == -1)
           {
                printf(1,"failed in exec2 \n");
                printf(1,"\n");
           }
          
           exit();
        }
       

    }

    //memlim cmd
    else if(strcmp("memlim",tmp)==0)
    {

        //get pid from input
        index = index +1;
        index2 = find_space(command+index);
        strcpyn(tmp,command+index,index2);

        //when first character is minus
        if(tmp[0] == '-')
            pid = -1* atoi(tmp+1);
        else
            pid =  atoi(tmp);

        //get limit from input
        index = index + index2+1;
        index2 = find_space(command+index);
        strcpyn(tmp,command+index,index2);

        
        if(tmp[0] == '-')
            limit = -1* atoi(tmp+1);
        else
            limit = atoi(tmp);

        if(setmemorylimit(pid,limit) == 0)
            printf(1,"succes in setting pid %d memory limit %d \n",pid, limit);
        else
            printf(1,"failed in setting pid %d memory limit \n",pid);
        printf(1,"\n");
    }

    //exit cmd
    else if(strcmp("exit",tmp)==0)
    { 
        break;
    }
    else
    {
        printf(1,"wrong input\n");
        printf(1,"\n");
    }

  }

  //reap all children
  while((pid=wait())!=-1);

  printf(1,"bye~\n");
  printf(1,"\n");
  exit();
}


// find space, enter ,null and  return that index
int find_space(char *s1)
{
    int index = 0;
    while(*s1 != ' ' &&  *s1 != '\0' && *s1 != 10)
    {
        s1 ++;
        index ++;
    }
    return index;
}
void strcpyn(char * dest, char * origin,int n)
{
    int i;
    for(i = 0 ; i < n ;i ++ )
    {
        *(dest+i) = *(origin+i);
    }

    // set null
    *(dest+i) = 0;

}
