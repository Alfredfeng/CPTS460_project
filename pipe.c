int show_pipe()
{
  PIPE *p = &pipe;
  int i;
  printf("----------------------------------------\n");
  printf("room=%d data=%d buf=", p->room, p->data);
  for (i=0; i<p->data; i++)
    kputc(p->buf[p->tail+i]);
  printf("\n");
  printf("----------------------------------------\n");
}
int kpipe()
{
  int i;
  PIPE *p = &pipe;
  p->head = p->tail = 0;
  p->data = 0; p->room = PSIZE;
  p->n_writer = p->n_reader = 1; //initialize the number of readers and writers to 1
}

int read_pipe(PIPE *p, char *buf, int n)
{
  int ret;
  char c;
  
  if (n<=0)
    return 0;
  show_pipe();

  while(n){
    printf("reader %d reading pipe\n", running->pid);
    ret = 0;
    while(p->data){
        *buf = p->buf[p->tail++];
        p->tail %= PSIZE;
        buf++;  ret++; 
        p->data--; p->room++; n--;
        if (n <= 0)
            break;
    }
    show_pipe();
    if (ret){   /* has read something */
       kwakeup(&p->room);
       return ret;
    }
    // pipe has no data
    printf("reader %d sleep for data\n", running->pid);
    kwakeup(&p->room);
    ksleep(&p->data);
    continue;
  }
}

int write_pipe(PIPE *p, char *buf, int n)
{
  char c;
  int ret = 0; 
  show_pipe();
  while (n){
    printf("writer %d writing pipe\n", running->pid);
    //checks broken pipe
    if(p->room == 8 && p->n_reader == 0)
    {
      printf("Detected a broken pipe!\n");
      return -1;
    }
    while (p->room){ //while there are rooms for writing
       p->buf[p->head++] = *buf; 
       p->head  %= PSIZE;
       buf++;  ret++; 
       p->data++; p->room--; n--;
       if (n<=0){
         show_pipe();
      	 kwakeup(&p->data);//wakes up the reader
      	 return ret;
       } //end if
    }// en inner while(p->room)
    show_pipe();
    printf("writer %d sleep for room\n", running->pid);
    kwakeup(&p->data);
    ksleep(&p->room);
  }
}
 
int pipe_reader()
{
  char line[128];
  int nbytes, n;
  PIPE *p = &pipe;
  printf("proc %d as pipe reader\n", running->pid);
 
  printf("input nbytes to read : " );
  nbytes = geti();
  n = read_pipe(p, line, nbytes); // reading bytes from the buffer
  if(n == 0)
  {
    printf("No more bytes to read: the reader exits\n");
    p->n_reader --;//decrement the number of readers
    kexit(0);
  }
  line[n] = 0;
  printf("Read n=%d bytes : line=%s\n", n, line);
}


int pipe_writer()
{
  char line[128];
  int nbytes, n;
  PIPE *p = &pipe;
  printf("proc %d as pipe writer\n", running->pid);

  printf("input a string to write \nan empty string will terminate the writer process:" );

  kgets(line);
  line[strlen(line)] = 0;

  if (strcmp(line, "")==0)
  {
    //an empty string kills the writer process
    pipe->n_writer --;//decrement the number of writers
    kexit(0); //kill the current running process
  }
  else{
    nbytes = strlen(line);
    printf("nbytes=%d buf=%s\n", nbytes, line);
    n = write_pipe(p, line, nbytes);
    printf("wrote n=%d bytes\n", n);
  }
}


