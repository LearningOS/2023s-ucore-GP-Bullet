#ifndef PROC_H
#define PROC_H

#include "types.h"

#define NPROC (16)
#define MAX_SYSCALL_NUM  (500)

// Saved registers for kernel context switches.
struct context {
	uint64 ra;
	uint64 sp;

	// callee-saved
	uint64 s0;
	uint64 s1;
	uint64 s2;
	uint64 s3;
	uint64 s4;
	uint64 s5;
	uint64 s6;
	uint64 s7;
	uint64 s8;
	uint64 s9;
	uint64 s10;
	uint64 s11;
};

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

//每个进程状态
struct proc {
	enum procstate state;//进程状态
	int pid;//进程号
	uint64 ustack;//用户栈的虚拟地址
	uint64 kstack;//内核栈的虚拟地址
	struct trapframe *trapframe;//trampoline.S 的数据页
	struct context context; // 用于保存进程内核态的寄存器信息，进程切换时使用
	/*
	* LAB1: you may need to add some new fields here
	*/

	uint64 syscalltimes[MAX_SYSCALL_NUM];
	uint64 begintime;
};

/*
* LAB1: you may need to define struct for TaskInfo here
*/

typedef enum {
    UnInit,
    Ready,
    Running,
    Exited,
} TaskStatus;

struct TaskInfo {
    TaskStatus status;
    unsigned int syscall_times[MAX_SYSCALL_NUM];
    int time;
};


struct proc *curr_proc();
void exit(int);
void proc_init();
void scheduler() __attribute__((noreturn));
void sched();
void yield();
struct proc *allocproc();
// swtch.S
void swtch(struct context *, struct context *);

#endif // PROC_H