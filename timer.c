//timer functions

TQE tqe[8]; //address of available tqe
TQE *timer_queue;// pointer to the head of timer queue

int strcpy_helper(char *dest, char *src)
{
  while(*src){
    *dest++ = *src++;
  }
  *dest = 0;
}

void timer_init()
{
	int i; TIMER *tp;
	TQE *tq = timer_queue;
	tq = 0;

	printf("timer_init()\n");
	for (i=0; i<4; i++){
		tp = &timer[i];
		if (i==0) tp->base = (u32 *)0x101E2000;
		if (i==1) tp->base = (u32 *)0x101E2020;
		if (i==2) tp->base = (u32 *)0x101E3000;
		if (i==3) tp->base = (u32 *)0x101E3020;
		*(tp->base+TLOAD) = 0x0;
		// reset
		*(tp->base+TVALUE)= 0xFFFFFFFF;
		*(tp->base+TRIS) = 0x0;
		*(tp->base+TMIS) = 0x0;
		*(tp->base+TLOAD) = 0x100;
		// CntlReg=011-0010=|En|Pe|IntE|-|scal=01|32bit|0=wrap|=0x66
		*(tp->base+TCNTL) = 0x66;
		*(tp->base+TBGLOAD) = 0x1C00; // timer counter value
		tp->tick = tp->hh = tp->mm = tp->ss = 0; // initialize wall clock

		strcpy_helper((char *)tp->clock, "00:00:00");
	}
}

void timer_start(int n) // timer_start(0), 1, etc.
{
	TIMER *tp = &timer[n];
	kprintf("timer_start %d base=%x\n", n, tp->base);
	*(tp->base+TCNTL) |= 0x80; 	// set enable bit 7
}

void timer_handler(int n) {
	int i;
	TIMER *t = &timer[n];
	TQE *head = timer_queue;
	t->tick++;
	
	// Assume 120 ticks per second
	if (t->tick==10){
		t->tick = 0; t->ss++;
		if(head)
		{
			update(1);//update the Timer queue
		}

		if (t->ss == 60){
			t->ss = 0; t->mm++;
			if (t->mm == 60){
				t->mm = 0; t->hh++; // no 24 hour roll around
			}
		}
		
	}
	if(t->tick == 0){
		t->clock[7]='0'+(t->ss%10); t->clock[6]='0'+(t->ss/10);
		t->clock[4]='0'+(t->mm%10); t->clock[3]='0'+(t->mm/10);
		t->clock[1]='0'+(t->hh%10); t->clock[0]='0'+(t->hh/10);
	}


	color = CYAN;
	// display in different color
	for (i=0; i<8; i++){
		kpchar(t->clock[i], n, 70+i); // to line n of LCD
	}
	timer_clearInterrupt(n); // clear timer interrupt
}

void addElement(int time)
{
	//implementation of
	TQE **first = &timer_queue;
	TQE **temp;
	TQE *head = *first;

	TQE *prev = head;//get the address of the head
	TQE *current = head; // get the address of timer_queue

	int pid = running->pid;

	//printf("Adding element in timer queue\n");

	tqe[pid].remaining_time = time;//get the remaining time
	tqe[pid].proc_pid = pid;
	tqe[pid].next = 0;

	TQE *t = &tqe[pid];
	int sleep_event = pid;

	//printf("t->remaining_time = %d\n",tqe[pid].remaining_time);
	if(! *first)
	{
		//add the element to the head
		//printf("Adding first element at the front\n");

		//printf("&proc[pid] = %d\n",&proc[pid]);
		*first = t;

		printTimerQueue();
		ksleep(sleep_event);
		return;
	}

	if( time - head->remaining_time <= 0)
	{
		//printf("Adding the new element in the front\n");
		*first = t;
		t->next = current;
		
		while(current)
		{
			current->remaining_time -= time;
			current = current->next; //advance pointer
		}

		printTimerQueue();
		ksleep(sleep_event);
		return;
	}
	

	

	//insertion in the middle
	while(current)
	{
		if(!current)
			break;
		if(time - current->remaining_time <= 0)
		{
			//printf("Adding element in the middle\n");
			temp = &prev;
			t->next = prev->next;
			(*temp)->next = t;

			prev = t;
			current = t->next;

			while(current)
			{
				current->remaining_time -= prev->remaining_time;
				prev = current;
				current = current->next;//advance pointer
			}
			printTimerQueue();
			ksleep(sleep_event);
			return;
		}
		time -= current->remaining_time;
		t->remaining_time = time;//update time
		prev = current;
		current = current->next;//advance pointer
	}
	//append at the end
	//printf("Adding element at the end\n");
	temp = &prev;
	(*temp)->next = t;
	printTimerQueue();

	ksleep(sleep_event);
	

}

void update(int tick)
{
	//this function gets called every second
	//printf("updating timer queue\n");
	TQE *head = timer_queue;
	TQE **temp = &timer_queue;

	int sleep_event;

	if(!head)
		return;
	sleep_event = head->proc_pid;
	//printf("head->prod_pid = %d\n",sleep_event);

	printTimerQueue();
	if(head->remaining_time == 0)
	{
		printf("waking up process %d\n", head->proc_pid);
		*temp = head->next;//update the head
	}
	kwakeup(sleep_event);//wake up the process
	head->remaining_time -= tick;
}

void printTimerQueue()
{
	TQE *head = timer_queue;
	
	while(head)
	{
		printf("[ %d , %d ]->",head->remaining_time, head->proc_pid);
		head = head->next;//advance pointer
	}
	printf("NULL\n");
}


int timer_clearInterrupt(int n) // timer_start(0), 1, etc.
{
	TIMER *tp = &timer[n];
	*(tp->base+TINTCLR) = 0xFFFFFFFF;
}


void timer_stop(int n)
{
	// stop a timer
	TIMER *tp = &timer[n];
	*(tp->base+TCNTL) &= 0x7F; // clear enable bit 7
}