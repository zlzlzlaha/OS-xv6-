#include "types.h"
#include "stat.h"
#include "user.h"

int 
main(int argc, char *argv[])
{
    
    int a = fork();
    int i;
    for(i = 0 ; i < 10 ; i++)
    {
        if( a != 0) {
            printf(1,"Parent\n");
            yield();
        }
        else {
            printf(1, "Child\n");
            yield();
        }
    }
    exit();
}
