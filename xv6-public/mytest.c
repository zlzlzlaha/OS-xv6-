#include "types.h"
#include "stat.h"
#include "user.h"
#define N 4096

// test fork stack
void f(int depth)
{
  volatile char arr[1000] = {0};
  sleep(100);
  printf(1, "pid %d ,Recursion depth: %d\n",getpid(), depth + arr[0]);
  f(depth + 1);
}

int main(int argc, char *argv[])
{
  fork();
  f(1);
  exit();
  return 0;
}


// basin test of shmem
/*
int main(int argc, char *argv[])
{
  int pid, parent;
  char *shmem;
  parent = getpid();
 
  printf(1,"parent pid %d \n",parent);
  if ((pid = fork()) < 0)
  {
    printf(1, "fork failed!\n");
    exit();
  }
  if (pid == 0)
  {
    shmem = getshmem(getpid());
    strcpy(shmem, "This is a message from the first child process");
    //printf(1, "The child process should still be able to write.\n");
    // strcpy(shmem, "Another message from the child process");
    sleep(150);
    strcpy(shmem, "Another message from the first child process");
    parent = getpid();

     pid = fork();
     if(pid == 0)
     {
        shmem = getshmem(parent);
        printf(1," I am second child A message from first child process : %s\n",shmem);
        
       // strcpy(shmem, "Another message from second child process \n");
       // printf(1,"this should not be print\n");
        

        shmem =getshmem(getpid());
        strcpy(shmem, "This is a message from the second child process");
        sleep(1000);
        shmem =getshmem(parent);
        strcpy(shmem, "This is a message from the second child process");
        printf(1,"this message should not be printed!\n");
        
     }
     else
     {
         sleep(2000);
     }
  }
  else
  {
    sleep(100);
    shmem = getshmem(pid);
    printf(1, "I am most parent A message from the child process: %s\n", shmem);
    sleep(500); 

    printf(1, "I am most parent A message from the child process: %s\n", shmem);
    shmem = getshmem(pid+1);
    printf(1, "I am most parent A message from the second process: %s\n", shmem);
 
    strcpy(shmem, "The parent process is not allowed to write in the shared memory!");
    printf(1, "this should not be printed\n");
    
    
  }
  

  exit();
  return 0;
}
*/
/*
//test for deallocate 
int main(int argc, char *argv[])
{
  int pid, parent;
  char *shmem;
  parent = getpid();
 
  printf(1,"parent pid %d \n",parent);
  if ((pid = fork()) < 0)
  {
    printf(1, "fork failed!\n");
    exit();
  }
  if (pid == 0)
  {
    shmem = getshmem(getpid());
    strcpy(shmem, "This is a message from the first child process");
        parent = getpid();
    sleep(300);
  }
  else
  {
    sleep(100);
    shmem = getshmem(pid);
    sleep(500);
    //strcpy(shmem, "This is a message from the from most parent process");
    printf(1, "I am most parent A message from the child process: %s\n", shmem);
    printf(1,"this should not be printed\n");    
    
  }
  

  exit();
  return 0;
}

*/

/*
// test fork limit 
int main(int argc, char *argv[])
{
  int total = 0;
  int pid ;

  printf(1,"parent pid %d\n",getpid());

  getadmin("2016025687");
  setmemorylimit(getpid(),50000);
  pid = fork();
  if(pid == 0){
    printf(1,"I'am child pid %d\n ",getpid());
    for (;;)
    {
        if (malloc(N) == 0)
        {
         printf(1, "Memory allocation failed.\n");
        exit();
        }
        total += N;
        printf(1, "Memory allocated: %d bytes\n", total);
        sleep(300);
  }
    exit();
  }
  exit();
  return 0;
}
*/



