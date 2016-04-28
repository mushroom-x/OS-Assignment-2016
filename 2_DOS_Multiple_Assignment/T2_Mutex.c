/**
 * Thread 1 : Print a
 * Thread 2 : Print b
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>


/* ------------------------DEFINE---------------------- */
/* DOS */
#define GET_INDOS 0x34
#define GET_CRIT_ERR 0x5d06

/* BUFF */
#define MAX_TEXT_LEN 20	
#define MAX_BUF_NUM 5

/* TCB */
#define MAX_TCB_NUM  5	

/* Thread */
#define FINISHED 0  			
#define RUNNING 1  				
#define READY 2  				
#define BLOCKED 3  

#define STACK_SIZE 1024

/* ------------------------DEFINE---------------------- */


/* -------------------------Struct-------------------------*/
/* ? */
typedef int (far *codeptr)();

typedef struct buffer	 buffer;
typedef struct TCB 		 TCB;
typedef struct semaphore semaphore;
typedef struct int_regs  int_regs;


struct buffer
{
    int sender;  				
    int size;  					
    char text[MAX_TEXT_LEN];
    struct buffer *next;
};

/* Semaphore - Record Type*/
struct semaphore
{
    int value;              /* Semaphore value */
    struct TCB *wq;         /* Top Pointer of Block Queue*/
};

/* Thread Control Block */
struct TCB
{
    unsigned char *stack; 		
    unsigned ss;  				
    unsigned sp; 				
    char state; 			 
    char name[10];  			
    struct TCB *next;  			
    struct buffer *mq;  		
    
    semaphore mutex;  			
    semaphore sm;  				
};


/* private stack of thread*/
struct int_regs
{
    unsigned BP, DI, SI, DS, ES, DX, CX, BX, AX, IP, CS, Flags, off, seg;
};


/* -------------------------Struct-------------------------*/



/*-----------------------Function Define ----------------------- */

/* ADD */		
void    T1AddOne();			
void 	T2AddTwo();


/* DOS */
void 	DosInit();
int 	DosBusy();

/* TIME */

void 	interrupt new_int8();		
void 	interrupt my_swtch();
void 	my_delay(int n);


/* TCB */
void 	TcbInitAll();
void 	TcbInit();				
void 	TcbState();

/* THREAD */
int  	ThreadCreate(char *name,codeptr code,int stackLen);
int 	ThreadCreateAt(int id,char *name,codeptr code,int stackLen);
int  	ThreadNext();					
int  	ThreadFinished();				
void 	ThreadDestory(int id);			
void 	ThreadDestoryAuto();			
void 	ThreadBlock(TCB **blkq); 		
void 	ThreadWakeUp(TCB **blkq);	

/* BUF */
void  	BufInitAll();
void  	BufInit(buffer * bufNew);
void  	BufPut(buffer * bufNew);
void 	BufInsertMsgq(buffer **msgq,buffer *bufNew);
buffer 	*BufGet();
buffer 	*BufRemov(buffer **msgq,int sender);

/* Semaphore */
void SemInit();
void P(semaphore *sem);
void V(semaphore *sem);

/*------------------------Function Define ----------------------*/


/*------------------------ Variable ----------------------------*/
/* TIME */
int timeCount = 0; 				
int timeSlice = 1;
void interrupt (*old_int8)();

/* SEMAPHORE */


/* DOS */
char far *indosPtr = 0;         
char far *critErrPtr = 0;  

/* TCB */
TCB tcbs[MAX_TCB_NUM];

/* BUF*/
buffer * freebuf;

/* Thread*/
int current;

/* Add To Sum*/
semaphore mutex = {1,NULL};

int sum = 0;
/*------------------------ Variable ----------------------------*/



int main()
{
	/* Initialization Part */
	DosInit();
	TcbInitAll();
    BufInit(freebuf);
  
	/* Get access to the entry address of clock intrrupt */
	old_int8 = getvect(8);

    /*
	printf("Please Input the Time Slice \n");
	scanf("%d",&timeSlice);

	printf("Press Any Key To Continue ......\n");
	getch();
    */
    
    clrscr();
    
    tcbs[0].state = RUNNING;
    current = 0;    /* current thread */
    sum = 0;

    ThreadCreate("T1",(codeptr)T1AddOne,STACK_SIZE);
	ThreadCreate("T2",(codeptr)T2AddTwo,STACK_SIZE);
	
        
    setvect(8, new_int8);
    my_swtch();

    TcbState();
    while(!ThreadFinished());

    TcbState();
    setvect(8, old_int8); /* Recover old clock interrupt */
    printf("End : Sum is %d \n",sum);
    ThreadDestory(0);
    return 0;
}

void T1AddOne()
{
    int i;
    int tmp;

    for(i=0; i<50; i++)
    {
        P(&mutex);
        tmp = sum;
        my_delay(1);
        tmp++;
        sum = tmp;
        printf("#1 : sum = %d \n", sum);
        V(&mutex);
    }
}

void T2AddTwo()
{
    int i;
    int tmp;

    for(i=0; i<50; i++)
    {
        P(&mutex);
        tmp = sum;
        delay(5);
        tmp = tmp + 2;
        sum = tmp;
        printf("#2 : sum = %d \n", sum);
        V(&mutex);
    }
}
/*------------------------ DOS ----------------------- */
/**
 * [DosInit description]
 */
void DosInit()
{
    union REGS regs;
    struct SREGS segregs;
    regs.h.ah = GET_INDOS;
    intdosx(&regs, &regs, &segregs);  
    indosPtr = MK_FP(segregs.es, regs.x.bx);  
   
    if(_osmajor < 3)
        critErrPtr = indosPtr + 1;  
    else if(_osmajor == 3 && _osminor == 0)
        critErrPtr = indosPtr - 1;
    else
    {
        regs.x.ax = GET_CRIT_ERR;
        intdosx(&regs, &regs, &segregs);
        critErrPtr = MK_FP(segregs.ds, regs.x.si);
    }
}

/**
 * [DosBusy : Check if Dos is busy ]
 * @return [isBusy]
 */
int DosBusy()
{
    if(indosPtr && critErrPtr)
        return (*indosPtr || *critErrPtr);  
    else
        return -1;
}


/*------------------------ DOS ----------------------- */


/*-----------------   Time ----------------------- */

/**
 * new_int8 :  CPU scheduling caused by Time Slice
 * @return [description]
 */
void interrupt new_int8()
{
    (*old_int8)(); 
    timeCount++; 

    if(timeCount < timeSlice || DosBusy())  
        return;

    my_swtch();
}

/**
 * my_swtch :　CPU scheduling caused by another reason
 * @return [description]
 */
void interrupt my_swtch()
{
    int id;

    id = ThreadNext();
    if(id < 0)
    {
        printf("No Thread is Running\n");
        return;
    }

    disable();  
   
    tcbs[current].ss = _SS;
    tcbs[current].sp = _SP;
    if(tcbs[current].state == RUNNING)
        tcbs[current].state = READY;

        /*id = 0;*/

    _SS = tcbs[id].ss;
    _SP = tcbs[id].sp;
    tcbs[id].state = RUNNING;
    current = id;
    timeCount = 0;  
    enable();
}

/**
 * my_delay : do nothing but delay time
 */

void my_delay(int n)
{
    int i;
    int j;
    int k;
    for(k = 0; k < n; k ++)
    {
        for(i = 0; i < 100; i++)
        {
            for(j = 0; j < 500; j++)
            {
                /*Do Nothing*/
            }
        }
    }
    
}

/*------------------------ Time ----------------------- */


/*------------------------ Tcb ----------------------- */
/**
 * [TcbInitAll :Init All Tcbs]
 */
void TcbInitAll()
{
    int id;
    for(id = 0; id < MAX_TCB_NUM; id++)
    {
       TcbInit(id);
    }
}


/**
 * [TcbInit : Init Single Tcb]
 * @param id [Tcb id]
 */
void TcbInit(int id)
{
    tcbs[id].stack = NULL;
    tcbs[id].state = FINISHED;
    tcbs[id].name[0] = '\0';
    tcbs[id].next = NULL;
    tcbs[id].mq = NULL;
    tcbs[id].mutex.value = 1;
    tcbs[id].mutex.wq = NULL;
    tcbs[id].sm.value = 0;
    tcbs[id].sm.wq = NULL;
}

/**
 * [TcbState :print current tcb state]
 */
void TcbState()
{
	int id;
    printf("\n ----- Current Thread Process -------\n");
    for(id = 0; id < MAX_TCB_NUM; id++)
    {
        printf("Thread %d %9s State is ", id, tcbs[id].name);
        switch(tcbs[id].state)
        {
            case FINISHED: 
            	puts("FINISHED"); 
            	break;
            case RUNNING: 
            	puts("RUNNING"); 
            	break;
            case READY: 
            	puts("READY");
            	break;
            case BLOCKED:
            	puts("BLOCKED"); 
            	break;
        }
    }

}

/*------------------------ Tcb ----------------------- */

/*------------------------ Thread ----------------------- */
/**
 * [ThreadCreate 创建新的线程]
 * @param  name    [线程的名字]
 * @param  code    [函数代码的首行地址]
 * @param  stckLen [堆栈的大小]
 * @return         [Tcb的ID]
 */
int ThreadCreate(char *name,codeptr code,int stackLen)
{
    int id;
    disable();

    for(id = 1; id < MAX_TCB_NUM ;id++)
    {
        if(tcbs[id].state == FINISHED)
        {
            break;
        }
    }
    if(id == MAX_TCB_NUM)
    {
        printf("\n Fail To Create %s \n", name);
        return -1;
    }

   	ThreadCreateAt(id,name,code,stackLen);

    enable();
    return id;
}

/**
 * [ThreadCreateAt description]
 * @param  id       [tcb id]
 * @param  name     [thread name]
 * @param  code     [function code address]
 * @param  stackLen [stack length]
 * @return          [flag -1:error]
 */
int ThreadCreateAt(int id,char *name,codeptr code,int stackLen)
{
	int_regs far *regs;

	if(tcbs[id].state == FINISHED)
	{   
   		tcbs[id].stack = (unsigned char *)malloc(stackLen);
		regs = (struct int_regs far *)(tcbs[id].stack + stackLen);
		regs--;
		tcbs[id].ss = FP_SEG(regs);
		tcbs[id].sp = FP_OFF(regs);
	    tcbs[id].state = READY;
	    strcpy(tcbs[id].name, name);

	    regs->DS = _DS;
	    regs->ES = _ES;
	    regs->IP = FP_OFF(code);
	    regs->CS = FP_SEG(code);
	    regs->Flags = 0x200;
	    regs->off = FP_OFF(ThreadDestoryAuto);
	    regs->seg = FP_SEG(ThreadDestoryAuto);
   
   		printf("\n Thread %d :  %s has been created \n", id, tcbs[id].name);
   		return 1;
	}
	else
	{
		printf("Error: Creating a thread on a running thread \n");
		return -1;
	}
}

/**
 * [ThreadDestory 线程在Tcb中的撤销]
 * @param id [Tcb Id]
 */
void ThreadDestory(int id)
{
    
    disable();
 
    free(tcbs[id].stack);
    TcbInit(id);
    printf("\n Thread %d :　%s already destory\n",id,tcbs[id].name);
    enable();
}

/**
 * [ThreadDestoryAuto :自动撤销当前线程]
 */
void ThreadDestoryAuto()
{
  
    ThreadDestory(current);
   
    my_swtch();
}

/**
 * [ThreadNext : 寻找下一个就绪线程]
 * @return [线程ID]
 */
int ThreadNext()
{
    int id;
    int i;

    /* TcbState(); */
    for(i = 1;i <= MAX_TCB_NUM;i++)
    {

        id = (current + i) % MAX_TCB_NUM;
        if(tcbs[id].state == READY || tcbs[id].state == RUNNING)
        {
            return id;
        }
    }

    return -1;
}

/**
 * [ThreadFinished 查看除了0#线程外的线程是否结束]
 * @return [isEnd]
 */
int ThreadFinished()
{
    int id;
    for(id = 1; id < MAX_TCB_NUM; id++)
    {
        if(tcbs[id].state != FINISHED)
        {
            return 0;
        }
    }
    return 1;
}

/**
 * [ThreadBlock :Insert current thread into Block Queue]
 * @param blkq [Block Queue]
 */
void ThreadBlock(TCB **blkq)
{
    TCB *tcbp;
    tcbs[current].state = BLOCKED;

    if((*blkq) == NULL)
    {
        (*blkq) = &tcbs[current];
    }
    else
    {
        tcbp = (*blkq);
        while(tcbp->next != NULL)
        {
            tcbp = tcbp->next;
        }
        tcbp->next = &tcbs[current];
    }

    tcbs[current].next = NULL;

    my_swtch();
}

/**
 * [ThreadWakeUp : Wake Up the Thread on the top of Block Queue]
 * @param blkq [Block Queue]
 */
void ThreadWakeUp(TCB **blkq)
{  
    struct TCB *tcbp;
    if((*blkq) == NULL)
        return;
    tcbp = (*blkq);
    (*blkq) = (*blkq)->next;
    tcbp->state = READY;
    tcbp->next = NULL;
}

/*------------------------ Thread ----------------------- */

/*------------------------ Buffer ----------------------- */
/**
 * [BufInitAll :Init whole buffer queue]
 */
void BufInitAll()
{
	struct buffer *bufPtr,*bufNew;
    int i;

    BufInit(freebuf);
    bufNew = freebuf;
    bufPtr = freebuf;

    for(i = 1; i < MAX_BUF_NUM; i++)
    {
        BufInit(bufNew);
        bufPtr->next = bufNew;
        bufPtr = bufNew;
    }

    bufPtr->next = NULL;
}

/**
 * [BufInit Init one buffer]
 * @param  bufNew [buffer pointer]
 * @return      [Null]
 */
void  BufInit(buffer *bufNew)
{
	bufNew = (buffer *)malloc(sizeof(buffer));
    bufNew->sender = -1;
    bufNew->size = 0;
    bufNew->text[0] = '\0';
}

/* Insert Message to Available Buffer Queue*/
/**
 * [BufPut : Insert Message to Available Buffer Queue]
 * @param  bufNew [new buffer]
 * @return        [Null]
 */
void  BufPut(buffer *bufNew)
{
  	struct buffer *bufPtr = freebuf;
    if(freebuf == NULL){
        freebuf = bufNew;
    }
    else 
    {
        while(bufPtr->next != NULL)
        {
            bufPtr = bufPtr->next;
        }

        bufPtr->next = bufNew;
    }
    bufNew->next = NULL;
}

/**
 * [BufInsertMsgq Insert Buffer to Message Queue ]
 * @param  msgq [message queue]
 * @param  bufNew [buffer queue header TODO]
 * @return      [Null]
 */
/* TODO */
void 	BufInsertMsgq(buffer **msgq,buffer *bufNew)
{
	buffer *bufPtr;

    if(bufNew == NULL)
    {
        return;
    }
    /* Message */ 
    if((*msgq) == NULL)
    {
        (*msgq) = bufNew;
    }
    else
    {
        bufPtr = (*msgq);
        while(bufPtr->next != NULL)
        {
            bufPtr = bufPtr->next;
        }

        bufPtr->next = bufNew;
    }

    bufNew->next  = NULL;
}

/**
 * [BufGet : Get Available Buffer Area]
 * @return [buffer]
 */
buffer *BufGet()
{
	buffer *bufNew;

    bufNew = freebuf;
    freebuf = freebuf->next;
    
    return bufNew;
}

/**
 * [BufRemov : Remove one buffer created by sender]
 * @param  msgq   [message queue]
 * @param  sender [sender id]
 * @return        [buffer]
 */
buffer *BufRemov(buffer **msgq,int sender)
{
	buffer *bufPtr, *bufNew;
    bufPtr = (*msgq);
   
    /* if the head of message queue then pop it*/
    if(bufPtr->sender == sender)
    {
        bufNew = bufPtr;
        (*msgq) = bufNew->next;
        bufNew->next = NULL;
        return bufNew;
    }

    /* Find buffer sent by sender */
    while(bufPtr->next != NULL && bufPtr->next->sender != sender)
        bufPtr = bufPtr->next;

    /* Can't find ,return null */
    if(bufPtr->next == NULL)
        return NULL;

    bufNew = bufPtr->next;
    bufPtr->next = bufNew->next;
    bufNew->next = NULL;
    return bufNew;
}
/*------------------------ Buffer ----------------------- */

/*------------------------ Semaphore ----------------------- */
/**
 * [SemInit : init the value of semaphore ]
 */
void SemInit()
{
    /*
	FreeBufMutexf = {1,NULL};			
	FreeBufNum   = {MAX_BUF_NUM,NULL};

	TcbMutex = {1,NULL};
	TcbEmpty = {MAX_TCB_NUM,NULL};
	TcbFull  = {0,NULL};	
    */
}

/**
 * [P : P Operation]
 * @param sem [semaphore pointer]
 */
void P(semaphore *sem)
{
    struct TCB **blkq;
    disable();

    sem->value--; 
    /* There is no available resources */
    if(sem->value < 0)
    {
        blkq = &(sem->wq);
        /* Insert current thread to Block Queue */
        ThreadBlock(blkq);
    }
    enable();
}

/**
 * [V : V operation]
 * @param sem [semaphore pointer]
 */
void V(semaphore *sem)
{
    struct TCB **blkq;
    disable();
    sem->value++;
    /* There are block threads in Block Queue */
    if(sem->value <= 0)
    {
        blkq = &(sem->wq);
        /* Wake Up the top thread from Block Queue*/
        ThreadWakeUp(blkq);
    }
    enable();
}

/*------------------------ Semaphore ----------------------- */
