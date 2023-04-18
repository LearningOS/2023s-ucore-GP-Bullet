#include "timer.h"
#include "riscv.h"
#include "sbi.h"

///读取“mtime”寄存器
uint64 get_cycle()
{
	return r_time();
}

///启用定时器中断
void timer_init()
{
	//启用监控定时器中断
	w_sie(r_sie() | SIE_STIE);
	set_next_timer();
}

///设置下一个定时器中断
void set_next_timer()
{
	const uint64 timebase = CPU_FREQ / TICKS_PER_SEC;
	set_timer(get_cycle() + timebase);
}