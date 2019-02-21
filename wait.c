int tswitch();

int ksleep(int event)
{
  int sr = int_off();
  //printf("proc %d going to sleep on event=%d\n", running->pid, event);

  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  //printList("sleepList", sleepList);
  tswitch();
  int_on(sr);
}

int kwakeup(int event)
{
  int num_woken = 0;
  PROC *temp, *p;
  temp = 0;
  int sr = int_off();
  
  while (p = dequeue(&sleepList)){
     if (p->event == event){
      	//printf("wakeup %d\n", p->pid);
      	p->status = READY;
      	enqueue(&readyQueue, p);
        num_woken ++;
     }
     else{
      	enqueue(&temp, p);
     }
  }
  sleepList = temp;
  int_on(sr);
  return num_woken;
}

int kexit(int exitValue)
{
  //printf("proc %d in kexit(), value=%d\n", running->pid, exitValue);
  if(running->pid == 1)
  {
    printf("P1 never dies\n");
    return -1;
  }
  running->exitCode = exitValue;
  running->status = ZOMBIE;
  kwakeup(running->parent);//wake up the parent
  //code to move all the children of the Zombie to the parent
  PROC *par_sibling = running;
  PROC *cur_sibling = running->child;
  while(par_sibling->sibling)
  {
    par_sibling = par_sibling->sibling;//advance sibling
  }
  par_sibling->sibling = cur_sibling;
  while(cur_sibling)
  {
    cur_sibling->ppid = running->ppid;
    cur_sibling = cur_sibling->sibling;//advance sibling
  }
  running->child = 0;//split the pointer

  //PROC* parent = running->parent;
  tswitch();
}

int kwait(int *status){
  //implementation of wait
  if(!running->child)
  {
    printf("Error: caller has no child\n");
    return -1;
  }
  while(1)
  {
    PROC *zombie = running->child;
    while(zombie)
    {
      if(zombie->status == ZOMBIE){
        //found a zombie child
        //get zombie child's pid
        *status = zombie->exitCode;//copy the zombie child's exitCode to *status
        zombie->status = FREE;//put it back to free list
        zombie->ppid = 0;//clear ppid
        zombie->parent->child = zombie->sibling;
        //dequeue(&readyQueue);
        enqueue(&freeList,zombie); //put it back to free list
        return zombie->pid;
      }
      zombie = zombie->sibling; // advance sibling
    }
    //has children but no zombie
    ksleep(&running);
  }
}


  
