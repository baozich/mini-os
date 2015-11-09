/*
 * traps.c
 */

#include <mini-os/os.h>
#include <mini-os/traps.h>

/*
 * TODO
 */
void stack_walk(void)
{
}

/*
 * TODO
 */
void do_bad_mode (struct pt_regs *regs, int reason)
{
	BUG();
}

void do_irq (struct pt_regs *regs)
{
}

/*
 * TODO
 */
void do_sync (struct pt_regs *regs)
{
}

/*
 * TODO: need implementation.
 */
void trap_init(void)
{
}

/*
 * TODO
 */
void trap_fini(void)
{
}
