# OS-xv6-
기존 XV6 코드를 수정하여, 운영체제에서 제공하는 스케줄링 / 메모리 관리 / 파일 시스템 관련된 기능을 추가/변경 하였다.

# Project1 Multi Level & MLFQ 스케줄링 
기존 xv6에서 round robin으로 동작하는 것을 수정하여 Multi Lelvel 스케줄링 방식 또는, MLFQ 방식으로 동작하게 변경하였다.   
2개의 스케줄링 방식은 make를 할 때, 옵션을 주어 변경할 수 있으며, MLFQ의 경우 Queue의 개수를 지정해줄 수 있다.
## Multi Level
Multi Level 스케줄링의 경우, 첫번째 level은 Round Robin 방식으로, 두번째 level은 FCFS 방식으로 동작한다.  
pid가 **짝수**인 프로세스들은 **round robin**으로 스케줄링하고, pid가 **홀수**인 프로세스들은 **FCFS**로 스케줄링된다.  
FCFS로 스케줄링되는 프로세스는 pid가 작은것부터 먼저 스케줄링 된다.   
Round Robin 프로세스 중에 RUNNABLE 한것이 존재하지 않을 때만 FCFS 큐를 확인한다. 단, Round Robin으로 스케쥴링을 받는 프로세스가 runnable한개 1개라도 있으면 다시 Round Robin으로 스케쥴링이 된다. 
### 선택한 자료구조  

Round Robin -> circular queue로 Round Robin이 동작하게 구현을 하였다. 
FCFS -> pid가 작은 프로그램부터 우선순위를 주기위해 priority queue를 이용하여 구현하였다.  

---

## MLFQ(Multi Level Feedback Queue)     

L0부터 Lk-1개의 k개의 큐로 동작하며 i번째 큐는  2i+4 tick의 time quantum을 가진다.  
큐의 숫자가 낮을 수록 우선 순위가 높은 큐이며 우선 순위가 높은 큐부터 스케줄링을 한다.
같은 큐 내에서도 우선 순위가 존재하며, 우선 순위가 높은 프로세스를 먼저 스케쥴링을 한다.   
time qunatum을 해당 큐에서 전부 소모하면, 하위 큐로 내려가게 된다. 
마지막 큐에서 모든 프로세스가 time quantum을 사용했거나, 100tick이 지나면 모든 프로세스를 L0 큐로 올리는 **priority booting**이 발생한다.

### 선택한 자료구조
Priority Queue를 사용하여 각 큐마다 1개씩 배정해주었다. 다만 스케줄링을 할때, 상위 큐부터 실행해야하는 프로세스가 있는지 확인해야 하는데, 스케줄링마다 만약 모든 프로세스가 하위큐에 있다면  O(n) n:프로세스 수 만큼 탐색을 해야하는 문제점이 발생한다. 이를 해결하기위해 모든 프로세스를 하나의 priority queue에서 관리하며, queue_level과 프로세스 우선 순위로 2가지의 값으로 스케줄링 순서를 정하는 시도를 하였다. 
구현과정에서 heapify 같은 연산을 할 때 오히려 많은 연산이 드는 점, 직접 코드로 구현을 했는데 매우 복잡하고 디버깅도 하기 힘들었었다... 구현 시간이 얼마 남아있지 않았던 터라, 구현을 했음에도 불구하고, 앞에서 말한 O(n)이 걸리는 알고리즘을 택했다. 당시 xv6에서 최대 프로세스가 64개로 define되어 있었지만, 더 많은 프로세스로 테스팅을 돌린다면 하나의 자료구조로 관리하는  방법도 고려해볼만 하다  

# Project2 Process Management

---
# Project3 User(File System Modification)
xv6에선 계정이라는 개념이 존재하지 않지만, 실제로 window나 linux는 사용자 계정이 존재한다. 
