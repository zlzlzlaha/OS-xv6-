# OS-xv6-
기존 XV6 코드를 수정하여, 운영체제에서 제공하는 스케줄링 / 메모리 관리 / 파일 시스템 관련된 기능을 추가/변경 하였다.

# Project1 Multi Level & MLFQ 스케줄링 
기존 xv6에서 round robin으로 동작하는 것을 수정하여 Multi Lelvel 스케줄링 방식 또는, MLFQ 방식으로 동작하게 변경하였다.  
2개의 스케줄링 방식은 make를 할 때, 옵션을 주어 변경할 수 있으며, MLFQ의 경우 Queue의 개수를 지정해줄 수 있다.  
Multi LEVEL 스케줄링의 경우, 첫번째 level은 Round Robin 방식으로, 두번째 level은 FCFS 방식으로 동작한다.  
process id가 짝수인 경우 RR 방식으로 동작하며, process id가 홀수 인 경우 FCFS 방식으로 동작한다.

# Project2 Process Management

# Project3 User(File System Modification)
xv6에선 계정이라는 개념이 존재하지 않지만, 실제로 window나 linux는 사용자 계정이 존재한다. 
