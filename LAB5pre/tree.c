//process binary tree and its operations
extern PROC *sleepList;
extern PROC *readyQueue;
extern PROC *tree;
extern char *pstatus[];

void addChild(PROC* proc)
{
	if(!running->child) //if the child pointer is empty, add the new process to the child
	{
		running->child = proc;
	}
	else
	{ //if the child pointer is not empty, append the newly created process to the end of the sibling
		PROC *pp = running->child;
		while(pp->sibling){
			pp = pp->sibling;
		}
		pp->sibling = proc;
	}
	proc->parent = running;
}

void printChild(PROC *proc) //proc should be running->child
{
	//implementation of printChild()
	PROC *pp = proc;
	printf("ChildList: ");
	if(!pp)
	{
		printf("NULL\n");
	}
	else
	{
		//print the first child
		printf("[%d , %s]->",pp->pid, pstatus[pp->status]);
		while(pp->sibling){ //while the parent has siblings
			printf("[%d, %s]->",pp->sibling->pid,pstatus[pp->sibling->status]);
			pp = pp->sibling;//advanced to sibling
		}
		printf("NULL\n");
	}
}