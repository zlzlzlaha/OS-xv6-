# OS-xv6-
기존 XV6 코드를 수정하여, 운영체제에서 제공하는 스케줄링 / 메모리 관리 / 파일 시스템 관련된 기능을 추가/변경 하였다.

# Project1 Multi Level & MLFQ 스케줄링 
기존 xv6에서 round robin으로 동작하는 것을 수정하여 Multi Lelvel 스케줄링 방식 또는, MLFQ 방식으로 동작하게 변경하였다.   
2개의 스케줄링 방식은 make를 할 때, 옵션을 주어 변경할 수 있으며, MLFQ의 경우 Queue의 개수를 지정해줄 수 있다.
## Multi Level
Multi Level 스케줄링의 경우, 첫번째 level은 Round Robin 방식으로, 두번째 level은 FCFS 방식으로 동작한다.  
pid가 **짝수**인 프로세스들은 **round robin**으로 스케줄링하고, pid가 **홀수**인 프로세스들은 **FCFS**로 스케줄링된다.  
FCFS로 스케줄링되는 프로세스는 pid가 작은것부터 먼저 스케줄링 된다.   
Round Robin 프로세스 중에 RUNNABLE 한것이 존재하지 않을 때만 FCFS 큐를 확인한다. 단, Round Robin으로 스케쥴링을 받는 프로세스가 runnable한개 1개라도 있으면 다시 Round Robin으로 스케줄링이 된다. 
### 선택한 자료구조  

Round Robin -> circular queue로 Round Robin이 동작하게 구현을 하였다. 
FCFS -> pid가 작은 프로그램부터 우선순위를 주기위해 priority queue를 이용하여 구현하였다.  


## MLFQ(Multi Level Feedback Queue)     

L0부터 Lk-1개의 k개의 큐로 동작하며 i번째 큐는  2i+4 tick의 time quantum을 가진다.  
큐의 숫자가 낮을 수록 우선 순위가 높은 큐이며 우선 순위가 높은 큐부터 스케줄링을 한다.
같은 큐 내에서도 우선 순위가 존재하며, 우선 순위가 높은 프로세스를 먼저 스케쥴링을 한다.   
time qunatum을 해당 큐에서 전부 소모하면, 하위 큐로 내려가게 된다. 
마지막 큐에서 모든 프로세스가 time quantum을 사용했거나, 100tick이 지나면 모든 프로세스를 L0 큐로 올리는 **priority booting**이 발생한다.

### 선택한 자료구조
Priority Queue를 사용하여 각 큐마다 1개씩 배정해주었다. 다만 스케줄링을 할때, 상위 큐부터 실행해야하는 프로세스가 있는지 확인해야 하는데, 스케줄링마다 만약 모든 프로세스가 하위큐에 있다면  O(n) n:프로세스 수 만큼 탐색을 해야하는 문제점이 발생한다. 이를 해결하기위해 모든 프로세스를 하나의 priority queue에서 관리하며, queue_level과 프로세스 우선 순위로 2가지의 값으로 스케줄링 순서를 정하는 시도를 하였다.  
구현과정에서 heapify 같은 연산을 할 때 오히려 많은 연산이 드는 점, 직접 코드로 구현을 했는데 매우 복잡하고 디버깅도 하기 힘들었었다... 구현 시간이 얼마 남아있지 않았던 터라, 구현을 했음에도 불구하고, 앞에서 말한 O(n)이 걸리는 알고리즘을 택하여 다시 구현했다. 당시 xv6에서 최대 프로세스가 64개로 define되어 있었지만, 더 많은 프로세스로 테스팅을 돌린다면 하나의 자료구조로 관리하는  방법도 고려해볼만 하다    

---

# Project2 Process Management  

## 프로세스 관리 system call  
프로세스 관리를 위한 추가적인 system call을 제공하고, 프로세스마다 서로가 공유할 수 있는 memory 영역 주소를 system call로 반환하였다.
기존의 exec과 똑같이 동작하지만 프로그램이 사용할 수 있는 stack size를 제한할 수 있는 **exec2** system call 구현   
선택한 pid의 메모리 크기의 제한을 걸 수 있게 하는 **setmemorylimit** system call 구현(malloc시 특정용량을 초과하면 malloc 호출에 실패하게 동작)    
프로그램이 실행되면, 프로그램이 실행된 시간을 프로세스 구조체에 기록하여 실행시간을 알 수 있음 
위에서 언급한 기능들을 shell처럼 명령어로 통제할 수 잇는 user program인 **pmanager** 구현 
  * execute <path> <stacksize>  : path의 경로에 위치한 프로램을 stack page 개수만큼 실행
  *  memlim <pid> <limit> : 할당 받을 수 있는 메모리를 제한
  *  exit :  pmanager 프로그램 종료 
  *  list : 현재 실행중인 프로세스들의 정보를 출력, pid, 실행시간, 스택용 페이지 개수, 할당받은 메모리 크기, 메모리 최대 제한 수를 출력 
  *  kill <pid> : 해당 pid 강제 종료   
  
## 공유 메모리   
프로세스마다 다른 프로세스와 메모리를 공유할 수 있는 페이지를 1개씩 가진다.  
메모리 주소는 **getshmem** system call을 호출함으로써, 선택한 pid의 공유 페이지 주소를 얻을 수 있다.   
자신의 공유 페이지에는 r/w가 모두 가능하지만, 다른 프로세스의 페이지에는 read만 가능하다  
이를 구현하기 위해, 프로세스가 fork가 되면 자신의 공유페이지를 kalloc으로 커널 영역 주소를 할당을 받으면, 모든 프로세스가 해당 페이지에 대한 page table을 업데이트 하도록 한다.  
대신 table에서 entry를 등록할때, 해당 페이지에 대한 권한 비트를 수정해야한다. 자신의 페이지 일경우 페이지 내용에 수정이 가능하지만, 다른 프로세스의 일 경우 해당 작업이 불가능 하도록 변경한다. 

---
# Project3 User(File System Modification)
xv6에선 계정이라는 개념이 존재하지 않지만, 실제로 window나 linux는 사용자 계정이 존재한다. 파일 시스템을 수정함으로써 해당 기능을 구현하였다. 
## 로그인 기능   
xv6를 실행하였을 때, 로그인을 하여 해당 계정으로 access control을 받을 수 있게 한다. 
원래 xv6에선 init -> sh -> user process 순으로 프로세스가 만들어지지만, init -> **login process** -> sh -> user process 순으로 프로세스가 실행 될 수 있게 init process를 수정하였다.   
login process에선 로그인에 성공해야 sh로 넘어갈 수 있으며, 로그인한 계정을 통해 파일들 권한을 체크한다. sh에서 logout을 입력하면 다시 login process로 돌아간다.   
로그인 정보를 관리하는 파일은 inode 단에서 지원하는 R/W 연산으로 계정 정보를 들고 있는 구조체 배열을 메모리에 쓰고, 읽어오는 방식으로 간단하게 구현하였다.  
## Access Control 
inode와 dinode쪽에 구조를 변경하여 파일에 소유자를 기록할 수 있게 하였으며, Onwer와 others에 대한 rwx 비트 값을 기록하였다.   
새로 만든 파일이 아닌 기존 xv6에서 기본적으로 제공하는 파일들은 root user와 기본 permissoin을 주게 커널 코드를 수정하였다. 
기존 xv6에서 지원하는 ls 프로세스는 어떤 종류의 파일인지(장치, 디렉토리, 파일), 파일 소유자와, 권한을 출력하지 못하는데, system call로 해당 정보에 접근할 수 있게 함으로써, 관련 내용을 출력한다.   
파일을 권한을 변경할 수 있는 **chmod system call**을 제공하며, 파일의 소유자나 root권한이 있는 user가 파일의 권한을 변경할 수 있다.  
파일과 디렉토리에 접근 하는 exec, open, creat, read, write 같은 system call 또는 그 내부에서 호출 하는 다른 함수가 rwx권한과 관련되는 작업을 할 때마다 직접 구현한 **check_mode** 함수호출하게 하여 권한을 제어한다. (구체적인 구현 내용은 당시에 남긴 문서로 확인 가능, project1 , project2 관련 내용도 터졌던 gitlab의 wiki를 복구시켜 docx로 남길까 고민...)





