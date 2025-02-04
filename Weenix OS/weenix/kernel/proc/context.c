/******************************************************************************/
/* Important Fall 2024 CSCI 402 usage information:                            */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/*         53616c7465645f5fd1e93dbf35cbffa3aef28f8c01d8cf2ffc51ef62b26a       */
/*         f9bda5a68e5ed8c972b17bab0f42e24b19daa7bd408305b1f7bd6c7208c1       */
/*         0e36230e913039b3046dd5fd0ba706a624d33dbaa4d6aab02c82fe09f561       */
/*         01b0fd977b0051f0b0ce0c69f7db857b1b5e007be2db6d42894bf93de848       */
/*         806d9152bd5715e9                                                   */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

#include "config.h"

#include "proc/context.h"
#include "proc/kthread.h"

#include "main/apic.h"
#include "main/interrupt.h"
#include "main/gdt.h"

#include "mm/page.h"
#include "mm/pagetable.h"

#include "util/debug.h"

static void
__context_initial_func(context_func_t func, int arg1, void *arg2)
{
        apic_setipl(IPL_LOW);
        intr_enable();

        void *result = func(arg1, arg2);
        kthread_exit(result);

        panic("\nReturned from kthread_exit.\n");
}

void
context_setup(context_t *c, context_func_t func, int arg1, void *arg2,
              void *kstack, size_t kstacksz, pagedir_t *pdptr)
{
        KASSERT(NULL != pdptr);
        KASSERT(PAGE_ALIGNED(kstack));

        c->c_kstack = (uintptr_t)kstack;
        c->c_kstacksz = kstacksz;
        c->c_pdptr = pdptr;

        /* put the arguments for __contect_initial_func onto the
         * stack, leave room at the bottom of the stack for a phony
         * return address (we should never return from the lowest
         * function on the stack */
        c->c_esp = (uintptr_t)kstack + kstacksz;
        c->c_esp -= sizeof(arg2);
        *(void **)c->c_esp = arg2;
        c->c_esp -= sizeof(arg1);
        *(int *)c->c_esp = arg1;
        c->c_esp -= sizeof(context_func_t);
        *(context_func_t *)c->c_esp = func;
        c->c_esp -= sizeof(uintptr_t);

        c->c_ebp = c->c_esp;
        c->c_eip = (uintptr_t)__context_initial_func;
}

void
context_make_active(context_t *c)
{
        gdt_set_kernel_stack((void *)((uintptr_t)c->c_kstack + c->c_kstacksz));
        pt_set(c->c_pdptr);

        /* Switch stacks and run the thread */
        __asm__ volatile(
                "movl %0,%%ebp\n\t"     /* update ebp */
                "movl %1,%%esp\n\t"     /* update esp */
                "push %2\n\t"           /* save eip   */
                "ret"                   /* jump to new eip */
                :: "m"(c->c_ebp), "m"(c->c_esp), "m"(c->c_eip)
        );
}

void
context_switch(context_t *oldc, context_t *newc)
{
        gdt_set_kernel_stack((void *)((uintptr_t)newc->c_kstack + newc->c_kstacksz));
        pt_set(newc->c_pdptr);

        /*
         * Save the current value of the stack pointer and the frame pointer into
         * the old context. Set the instruction pointer to the return address
         * (whoever called us).
         */
        __asm__ __volatile__(
                "pushfl           \n\t" /* save EFLAGS on the stack */
                "pushl  %%ebp     \n\t"
                "movl   %%esp, %0 \n\t" /* save ESP into oldc */
                "movl   %2, %%esp \n\t" /* restore ESP from newc */
                "movl   $1f, %1   \n\t" /* save EIP into oldc */
                "pushl  %3        \n\t" /* restore EIP */
                "ret              \n\t"
                "1:\t"                  /* this is where oldc starts executing later */
                "popl   %%ebp     \n\t"
                "popfl"                 /* restore EFLAGS */
                :"=m"(oldc->c_esp), "=m"(oldc->c_eip)
                :"m"(newc->c_esp), "m"(newc->c_eip)
        );
}
