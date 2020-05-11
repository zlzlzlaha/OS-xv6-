#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;

#ifdef MULTILEVEL_SCHED
struct queue {
  int qsize;
  int rear;
  int front;
  int capacity;
  struct proc *proc[NPROC];
};
struct priority_queue{
  int qsize;
  int capacity;
  struct proc *proc[NPROC+1];
};
struct ptmp{
    int index;
    struct proc * proc[NPROC];
};

struct queue rr;
struct priority_queue fcfs;
struct ptmp tmp;

void 
qinit(void)
{
  tmp.index = -1;
  rr.qsize = 0;
  rr.rear = 0;
  rr.front = 1;
  rr.capacity = NPROC;
  fcfs.qsize = 0;
  fcfs.capacity = NPROC;
}


struct proc*
depq(struct priority_queue * q)
{
  int i, child;

  struct proc * p = q->proc[1];
  struct proc * last = q->proc[q->qsize--];
  
  for(i =1 ; i *2 <= q->qsize ; i = child)
  {
   child = i*2;
   if((child != q->qsize) && (q->proc[child+1]->pid < q->proc[child]->pid))
   {
     child ++;
   }
   if(last->pid > q->proc[child]->pid)    
   {
     q->proc[i] = q->proc[child];
   }
   else 
     break;
  }
  q->proc[i] = last; 
  //cprintf("fcfs deque qsize %d \n",q->qsize);
  return p;
}

void 
enpq(struct proc * p, struct priority_queue * q)
{
  int i;
//  cprintf("enpq pid %d\n",p->pid);
  if(q->qsize == 0)
  {
      q->proc[++q->qsize] = p;
      return;
  }
  else if(q->qsize < q->capacity)   
  {
    for(i = ++q->qsize; (i/2!=0) &&((q->proc[i/2]->pid) > p->pid)  ; i/=2)
    {
      q->proc[i] = q->proc[i/2];
    }
    q->proc[i]  = p;
  }
}

struct proc*
deq(struct queue* q)
{
    struct  proc * p;
    p = q->proc[q->front];
    q->qsize--;
    q->front = (q->front+1) % q->capacity;
    return p;
}

void 
enq(struct proc * p, struct queue* q)
{
    //When queue is full.
    if(q->qsize == q->capacity)
        return;
    else
    {
        q->qsize++;
        q->rear = (q->rear+1) % q->capacity;
        q->proc[q->rear] = p; 
    }
}
#elif MLFQ_SCHED
struct priority_queue{
  int qsize;
  int capacity;
  struct proc *proc[NPROC+1];
};

struct ptmp{
    int index;
    struct proc dummy;
    struct proc * proc[NPROC];
};

struct priority_queue mlfq;
struct ptmp tmp;

void 
qinit(void)
{
  tmp.index = -1;
  tmp.dummy.state = UNUSED;
  mlfq.qsize  = 0;
  mlfq.capacity = NPROC;
}


struct proc*
deq(struct priority_queue * q)
{
  int i, child;

  struct proc * p = q->proc[1];
  struct proc * last = q->proc[q->qsize--];
  
  for(i =1 ; i *2 <= q->qsize ; i = child)
  {
   child = i*2;
   if((child != q->qsize) && (q->proc[child+1]->qlevel <= q->proc[child]->qlevel))
   {
     if( (q->proc[child+1]->qlevel < q->proc[child]->qlevel) || (q->proc[child+1]->priority > q->proc[child]->priority))
       child ++;
   }
   if(last->qlevel >= q->proc[child]->qlevel)    
   {
     if((last->qlevel > q->proc[child]->qlevel) || (last->priority < q->proc[child]->priority))
        q->proc[i] = q->proc[child];
     else
       break;
   }
   else 
     break;
  }
  q->proc[i] = last; 
  return p;
}

void 
enq(struct proc * p, struct priority_queue * q)
{
  int i;
  if(q->qsize == 0)
  {
      q->proc[++q->qsize] = p;
      return;
  }
  else if(q->qsize < q->capacity)   
  {
    for(i = ++q->qsize; (i/2!=0) &&(q->proc[i/2]->qlevel >= p->qlevel)  ; i/=2)
    {
      if(q->proc[i/2]->qlevel > p->qlevel || q->proc[i/2]->priority < p->priority)
        q->proc[i] = q->proc[i/2];
      else 
        break;
    }
    q->proc[i]  = p;
  }
}

void print_ptable()
{
    int i =0;
    for(i=0 ; i < 5;i++)
    {
        cprintf("pid %d \n",ptable.proc[i].pid);
    }
}
void
priority_boost(void)
{
  struct proc * p;

  p  = 0;
  while(mlfq.qsize >0)
  {
    tmp.proc[++tmp.index] = deq(&mlfq);
  }
  while(tmp.index >=0){
   p = tmp.proc[tmp.index--];
   if(p->qlevel >0){
     p->qlevel = 0 ;
     p->qtime = 4;
   }
   enq(p,&mlfq);
  }
}

#endif


extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  #ifdef MLFQ_SCHED
  p->priority = 0;
  p->qlevel = 0;
  p->qtime = 4;
  #endif
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;
     

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
    
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);
  p->state = RUNNABLE;
  
  #ifdef MULTILEVEL_SCHED
  if(p->pid %2 == 0)
    enq(p,&rr);
  else
    enpq(p,&fcfs);
  #elif MLFQ_SCHED
  enq(p,&mlfq);
  #endif  
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;
  
  #ifdef MULTILEVEL_SCHED
  
  if(np->pid %2 == 0)
    enq(np,&rr); 
  else{
    enpq(np,&fcfs); 
  }
  #elif MLFQ_SCHED
  enq(np,&mlfq);
  #endif
  


  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  // Multilevel scheduler.
  #ifdef MULTILEVEL_SCHED
  int cnt  = 0;
  c->proc = 0;
  cprintf("multilevel sched\n");
  for(;;){
    sti(); 
    acquire(&ptable.lock);        
    // Do RR scheduling
    cnt = 0;
     
      while(rr.qsize > 0){
        p = deq(&rr);
        if(p->state != RUNNABLE){
          if(p->state != UNUSED){
      //      cprintf("rr unused %d \n",p->pid);
            enq(p,&rr);
            cnt++;
          }
          if(cnt >= rr.qsize)
            break;
          continue;
        }
        cnt = 0;
      //  cprintf("rr pid %d \n",p->pid);
        enq(p,&rr);
        c->proc = p;
        switchuvm(p);
        p->state = RUNNING;
        swtch(&(c->scheduler), p->context);
        switchkvm();
        c->proc = 0;
        break;
     }
     // Do FCFS Scheuduler.
    if(cnt >= rr.qsize){
       while(fcfs.qsize > 0){
         p = depq(&fcfs);
         if(p->state != RUNNABLE){
           if(p->state != UNUSED){

        //     cprintf("not used pid %d \n ",p->pid);
             tmp.proc[++tmp.index] = p;
          //   cprintf("tmp qszie %d pid %d \n",tmp.index);
           }
           continue;
         }
        // cprintf("fcfs pid %d  q size%d \n",p->pid, fcfs.qsize);
         c->proc = p; 
         enpq(p,&fcfs);
         switchuvm(p);
         p->state = RUNNING;
         swtch(&(c->scheduler), p->context);
         switchkvm();
         c->proc = 0;
         break;
       }
       //cprintf("comeback here\n");
           
       while(tmp.index >= 0){
         enpq(tmp.proc[tmp.index--],&fcfs);
       }
       
       
    }
    release(&ptable.lock);
  }
  #elif MLFQ_SCHED
  cprintf("MLFQ_SCHED\n");
 
  c->proc = 0;
  
  int i =0;
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    while(mlfq.qsize > 0){
      p = deq(&mlfq);
      if(p->state != RUNNABLE  || (p->qtime < 0) ){
        if(p->state != UNUSED){
          tmp.proc[++tmp.index] = p;
        }
        continue;
      }
      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      for(i = 1 ; i <= mlfq.qsize ; i++)
      {
          if(mlfq.proc[i]->state == RUNNABLE  && mlfq.proc[i]->qlevel < p->qlevel)
              cprintf("wrong level :  current pid%d   level %d another pid %d level %d  \n",p->pid,p->qlevel,mlfq.proc[i]->pid,mlfq.proc[i]->qlevel);
          if(mlfq.proc[i]->state == RUNNABLE && mlfq.proc[i]->qlevel == p->qlevel && mlfq.proc[i]->priority > p->priority )
              cprintf("wrong level :  current p  priroity %d another  priority %d  \n",p->priority,mlfq.proc[i]->priority);
      }
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm(); 
      enq(p,&mlfq);

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
      break;
    }

    
    if(mlfq.qsize ==0 && tmp.index >0){
       priority_boost();  
    }
    else{
      while(tmp.index>=0)   
          enq(tmp.proc[tmp.index--],&mlfq);
    }
    release(&ptable.lock);
    
  }
  // Xv6 RR.
  #else
  c->proc = 0;
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);
  }
  #endif
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

#ifdef MLFQ_SCHED
// Set child proess priority.
int
setpriority(int pid, int priority)
{
   struct proc *p;
   int i;
  //Not proper priority range.
  if(priority < 0 || priority >10){
    return -2;
  }
//  acquire(&ptable.lock);
  
  //Check is this pid child.
/*
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(( pid == p->pid) && (p->parent->pid  == myproc()->pid) ){
    p->priority = priority;
      return 0;
    }   
  }
*/  
  
  while(tmp.index >-1){
   p = tmp.proc[tmp.index--];
   if((p->pid == pid) && (p->parent->pid == myproc()->pid)){
     p->priority = priority;
     enq(p,&mlfq);
     return 0;
   }
  }
  for(i = 1 ; i <= mlfq.qsize; i ++){
    p = mlfq.proc[i];
    if((p->pid == pid) && (p->parent->pid == myproc()->pid)){
      p->priority =priority;
      mlfq.proc[i] = &tmp.dummy;
      enq(p,&mlfq);
      return 0;
    }
  }
  //release(&ptable.lock);
  return -1;

}

// Get queue level which this proess is involved in.
int getlev(void)
{
  return myproc()->qlevel;
}
#endif 

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;
  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
