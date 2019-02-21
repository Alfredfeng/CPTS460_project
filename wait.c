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
  PROC *temp, *p;
  temp = 0;
  int sr = int_off();
  
  //printList("sleepList", sleepList);

  while (p = dequeue(&sleepList)){
     if (p->event == event){
      	//printf("wakeup %d\n", p->pid);
      	p->status = READY;
      	enqueue(&readyQueue, p);
     }
     else{
      	enqueue(&temp, p);
     }
  }
  sleepList = temp;
  //printList("sleepList", sleepList);
  int_on(sr);
}

int kexit(int exitValue)
{
  printf("proc %d in kexit(), value=%d\n", running->pid, exitValue);
  if(running->pid == 1)
  {
    printf("P1 never dies\n");
    return -1;
  }
  running->exitCode = exitValue;
  running->status = ZOMBIE;
  //code to move all the children of the Zombie to the parent
  //PROC* parent = running->parent;
  tswitch();
}

int kwait(){
  //implementation of wait
}


  
