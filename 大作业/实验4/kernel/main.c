/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

/*======================================================================*
                           新增队列
 *======================================================================*/

int num;

#define MAX_QUEUE_SIZE 10
typedef struct queue  
{    
    int front;  
    int rear;  
    int queue_array[MAX_QUEUE_SIZE] ;  
      
}SqQueue;  
SqQueue S;

SqQueue Init_Queue()            /*  队列初始化  */  
{      
    SqQueue S;  
    S.front=S.rear=0;    
    return (S);  
}  
int push(SqQueue *S,int e)      /*  使数据元素e进队列成为新的队尾  */  
{    
    (*S).queue_array[(*S).rear]=e;    /* e成为新的队尾  */  
    (*S).rear++ ;                     /*  队尾指针加1  */  
}  
int pop(SqQueue *S,int *e )     /*弹出队首元素*/  
{    
    *e=(*S).queue_array[(*S).front] ;    
    (*S).front++;    
}  


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
		proc_table[i].ticks = proc_table[i].isWait = 0;
	}


	k_reenter = 0;
	ticks = 0;

	//TODO
	waiting = 0;
	customers = 0;
	barbers = 0;
	mutex = 1;
	num = 0;

	/*mutex = 1;
	full = 0;
	empty = BUFFER_NUMBER;*/

	p_proc_ready	= proc_table;

        /* 初始化 8253 PIT */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

        put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

	disp_pos=0;
	for(i=0;i<80*25;i++){
		disp_str(" ");
	}
	disp_pos=0;

	S=Init_Queue();

	restart();

	while(1){}
}

/*=============
	P
===============*/
void P(int ID, int p)
{
	int i = sem_p(ID);
//	if(p == 1){
//		sys_disp_str("Barber waiting");
//		sys_disp_str("\n");			
//	}
	if(i < 0) {
		p_proc_ready->isWait = 1;
		push(&S,p);
		while(p_proc_ready->isWait){}
	}
}

/*=============
	V
==============*/
void V(int ID, int p)
{
	int i = sem_v(ID);
	if(i <= 0) {
		int t=0;
		pop(&S,&t);
		proc_table[t].isWait = 0;
	}
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()	//TODO 普通进程
{
	int i = 0;
	while (1) {
		sys_process_sleep(10);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()	//TODO 理发师
{
	int i = 0x1000;
	while(1){
		P(CUSTOMERS, 1);
		P(MUTEX, 1);
		waiting--;
		V(BARBERS, 1);
		V(MUTEX, 1);
		disp_color_str("barber is cuting hair\n", 4);
		my_milli_delay(2000);
	}
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()	//TODO 顾客A
{
	int i = 0x2000;
	while(1){
		//customerBehavior(2, 1);
		P(MUTEX, 2);
		sys_disp_str("Customer ");
		printInt(++num);
		sys_disp_str(" come\n");
		int i = num;
		if(waiting < CHAIRS){
			waiting++;
			//打印等待人数
//			printInt(waiting);
//			sys_disp_str(" people waiting\n");
			V(CUSTOMERS, 2);
			V(MUTEX, 2);
			P(BARBERS, 2);
			sys_disp_str("Customer ");
			printInt(i);
			sys_disp_str(" get haircutted\n");
			my_milli_delay(2000);
		} else {
			V(MUTEX, 2);
			sys_disp_str("no chairs\n");
			my_milli_delay(600);
		}
	}
}

/*======================================================================*
                               TestD
 *======================================================================*/
void TestD()	//TODO 顾客B
{
	int i = 0x3000;
	while(1){
		//customerBehavior(3, 1);
		P(MUTEX, 3);
		sys_disp_str("Customer ");
		printInt(++num);
		sys_disp_str(" come\n");
		int i = num;
		if(waiting < CHAIRS){
			waiting++;
			//打印等待人数
//			printInt(waiting);
//			sys_disp_str(" people waiting\n");
			V(CUSTOMERS, 3);
			V(MUTEX, 3);
			P(BARBERS, 3);
			sys_disp_str("Customer ");
			printInt(i);
			sys_disp_str(" get haircutted\n");
			my_milli_delay(2000);
		} else {
			V(MUTEX, 3);
			sys_disp_str("no chairs\n");
			my_milli_delay(600);
		}
	}
}

/*======================================================================*
                               TestD
 *======================================================================*/
void TestE(){	//TODO 顾客C
	int i = 0x4000;
	while(1){
		//customerBehavior(4, 1);
		P(MUTEX, 4);
		sys_disp_str("Customer ");
		printInt(++num);
		sys_disp_str(" come\n");
		int i = num;
		if(waiting < CHAIRS){
			waiting++;
			//打印等待人数
//			printInt(waiting);
//			sys_disp_str(" people waiting\n");
			V(CUSTOMERS, 4);
			V(MUTEX, 4);
			P(BARBERS, 4);
			sys_disp_str("Customer ");
			printInt(i);
			sys_disp_str(" get haircutted\n");
			my_milli_delay(2000);
		} else {
			V(MUTEX, 4);
			sys_disp_str("no chairs\n");
			my_milli_delay(600);
		}
	}
}

void printInt(int j){
	int nu = 0;
	int m = j;
	while(m>0){
		m = m/10;
		nu++;	
	}
	int tt;
	char ss[nu]; 
	for(tt=0;tt<nu;tt++){
		int temp = j%10;
		j = j/10;
		ss[tt] = '0'+ temp;
	}
	char mm[nu+1];
	for(tt=0;tt<nu;tt++){
		mm[tt] = ss[nu-tt-1];
	}
	mm[nu] = '\0';
	//char i = j + '0';
	//char s[] = {i, '\0'};
	sys_disp_str(mm);
}


