#include "proc.h"
#include "defs.h"
#include "loader.h"
#include "trap.h"
#include"time.h"

struct proc pool[NPROC];// 全局进程池

// 由于还有没内存管理机制，静态分配一些进程资源
char kstack[NPROC][PAGE_SIZE];
__attribute__((aligned(4096))) char ustack[NPROC][PAGE_SIZE];
__attribute__((aligned(4096))) char trapframe[NPROC][PAGE_SIZE];

extern char boot_stack_top[];
struct proc *current_proc;// 指示当前进程
struct proc idle;// boot 进程，执行初始化的进程

int threadid()
{
	return curr_proc()->pid;
}

struct proc *curr_proc()
{
	return current_proc;
}

//在启动时初始化 proc 表。
void proc_init(void)
{
	struct proc *p;
	for (p = pool; p < &pool[NPROC]; p++) {
		p->state = UNUSED;
		p->kstack = (uint64)kstack[p - pool];
		p->ustack = (uint64)ustack[p - pool];
		p->trapframe = (struct trapframe *)trapframe[p - pool];
		/*
		* LAB1: you may need to initialize your new fields of proc here
		*/
	}
	idle.kstack = (uint64)boot_stack_top;
	idle.pid = 0;
	current_proc = &idle;
}

int allocpid()
{
	static int PID = 1;
	return PID++;
}

//在进程表中查找未使用的进程。
//如果找到，则初始化在内核中运行所需的状态。
//如果没有空闲过程，或者内存分配失败，则返回 0。
struct proc *allocproc(void)
{
	struct proc *p;
	for (p = pool; p < &pool[NPROC]; p++) {
		if (p->state == UNUSED) {
			goto found;
		}
	}
	return 0;
	
//初始化其PID以及清空其栈空间，
//并设置 context 第一次运行的入口地址 usertrapret，
//使得进程能够从内核的S态返回U态并执行自己的代码
found:
	p->pid = allocpid();
	p->state = USED;
	memset(&p->context, 0, sizeof(p->context));
	memset(p->trapframe, 0, PAGE_SIZE);
	memset((void *)p->kstack, 0, PAGE_SIZE);
	p->context.ra = (uint64)usertrapret;
	p->context.sp = p->kstack + PAGE_SIZE;
	p->begintime = 0;
	memset(p->syscalltimes,0,sizeof(uint64)*MAX_SYSCALL_NUM);
	return p;
}

//调度程序永远不会返回。它循环，做：
//-选择要运行的进程。
//-切换到开始运行该进程。
//-最终该进程转移控制
//通过切换回调度程序。
void scheduler(void)
{
	struct proc *p;
	for (;;) {
		for (p = pool; p < &pool[NPROC]; p++) {
			if (p->state == RUNNABLE) {
				/*
				* LAB1: you may need to init proc start time here
				*/
				if(p->begintime== 0)
					p->begintime = get_cycle();
				p->state = RUNNING;
				current_proc = p;
				swtch(&idle.context, &p->context);
			}
		}
	}
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void)
{
	struct proc *p = curr_proc();
	if (p->state == RUNNING)
		panic("sched running");
	swtch(&p->context, &idle.context);
}

//放弃 CPU 进行一轮调度。
void yield(void)
{
	current_proc->state = RUNNABLE;
	sched();
}

// Exit the current process.
void exit(int code)
{
	struct proc *p = curr_proc();
	infof("proc %d exit with %d", p->pid, code);
	p->state = UNUSED;
	finished();
	sched();
}
