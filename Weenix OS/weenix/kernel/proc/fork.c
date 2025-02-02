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

#include "types.h"
#include "globals.h"
#include "errno.h"

#include "util/debug.h"
#include "util/string.h"

#include "proc/proc.h"
#include "proc/kthread.h"

#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/page.h"
#include "mm/pframe.h"
#include "mm/mmobj.h"
#include "mm/pagetable.h"
#include "mm/tlb.h"

#include "fs/file.h"
#include "fs/vnode.h"

#include "vm/shadow.h"
#include "vm/vmmap.h"

#include "api/exec.h"

#include "main/interrupt.h"

/* Pushes the appropriate things onto the kernel stack of a newly forked thread
 * so that it can begin execution in userland_entry.
 * regs: registers the new thread should have on execution
 * kstack: location of the new thread's kernel stack
 * Returns the new stack pointer on success. */
static uint32_t
fork_setup_stack(const regs_t *regs, void *kstack)
{
        /* Pointer argument and dummy return address, and userland dummy return
         * address */
        uint32_t esp = ((uint32_t) kstack) + DEFAULT_STACK_SIZE - (sizeof(regs_t) + 12);
        *(void **)(esp + 4) = (void *)(esp + 8); /* Set the argument to point to location of struct on stack */
        memcpy((void *)(esp + 8), regs, sizeof(regs_t)); /* Copy over struct */
        return esp;
}


/*
 * The implementation of fork(2). Once this works,
 * you're practically home free. This is what the
 * entirety of Weenix has been leading up to.
 * Go forth and conquer.
 */
int
do_fork(struct regs *regs)
{
        KASSERT(regs != NULL);
        KASSERT(curproc != NULL);
        KASSERT(curproc->p_state == PROC_RUNNING);
        dbg(DBG_PRINT, "(GRADING3A 7.a)\n");
        
        vmarea_t *vma, *clone_vma;
        pframe_t *pf;
        mmobj_t *to_delete, *new_shadowed;

        proc_t *child_proc = proc_create("child");

        

        vmmap_t *child_vmmap = vmmap_clone(curproc->p_vmmap);
        child_proc->p_vmmap = child_vmmap;
        child_proc->p_vmmap->vmm_proc = child_proc;
        child_vmmap->vmm_proc = child_proc;
        
        vmarea_t *curr;

        list_iterate_begin(&child_vmmap->vmm_list, curr, vmarea_t, vma_plink) {
                vmarea_t *parent_curr = vmmap_lookup(curproc->p_vmmap, curr->vma_start);
                mmobj_t *parent_mmobj = parent_curr->vma_obj;
                if(curr->vma_flags & MAP_SHARED){
                        curr->vma_obj = parent_mmobj;
                        curr->vma_obj->mmo_ops->ref(curr->vma_obj);   
                        dbg(DBG_PRINT, "(GRADING3D)\n"); 
                }

                else{
                        mmobj_t *shadow_mmobj;
                        mmobj_t *parent_shadow_mmobj;
                        shadow_mmobj = shadow_create();
                        parent_shadow_mmobj = shadow_create();

                        shadow_mmobj->mmo_shadowed = parent_mmobj;
                        parent_shadow_mmobj->mmo_shadowed = parent_mmobj;
                        parent_mmobj->mmo_ops->ref(parent_mmobj); 

                        parent_shadow_mmobj->mmo_un.mmo_bottom_obj = mmobj_bottom_obj(parent_mmobj);
                        shadow_mmobj->mmo_un.mmo_bottom_obj = mmobj_bottom_obj(parent_mmobj);
                        curr->vma_obj = shadow_mmobj;  
                        parent_curr->vma_obj = parent_shadow_mmobj;
                        dbg(DBG_PRINT, "(GRADING3A)\n"); 
                                                 
                }
                list_insert_head(mmobj_bottom_vmas(parent_curr->vma_obj), &(curr->vma_olink));
                dbg(DBG_PRINT, "(GRADING3A)\n");  

        } 
        list_iterate_end();

        kthread_t *child_thread = kthread_clone(curthr);
        child_thread->kt_proc = child_proc;
        list_insert_tail(&child_proc->p_threads, &child_thread->kt_plink);

        KASSERT(child_proc->p_state == PROC_RUNNING);
        KASSERT(child_proc->p_pagedir != NULL); 
        dbg(DBG_PRINT, "(GRADING3A 7.a)\n");

        regs->r_eax = 0;        
        child_thread->kt_ctx.c_eip = (uint32_t) userland_entry;
        child_thread->kt_ctx.c_pdptr = child_proc->p_pagedir;    
        uint32_t child_stack = fork_setup_stack(regs, child_thread->kt_kstack);  
        
        KASSERT(child_thread->kt_kstack != NULL);
        dbg(DBG_PRINT, "(GRADING3A 7.a)\n");

        child_thread->kt_ctx.c_esp = child_stack;
        child_thread->kt_ctx.c_kstack = (uintptr_t) child_thread->kt_kstack;
        child_thread->kt_ctx.c_kstacksz = DEFAULT_STACK_SIZE;

        for (int i = 0; i < NFILES; i++){                
                child_proc->p_files[i] = curproc->p_files[i];
                if (child_proc->p_files[i] != NULL){                                          
                        fref(child_proc->p_files[i]);
                        dbg(DBG_PRINT, "(GRADING3A)\n"); 
                }              
                dbg(DBG_PRINT, "(GRADING3A)\n");   
        }
  
        child_proc->p_brk = curproc->p_brk;
	child_proc->p_start_brk = curproc->p_start_brk;
        pt_unmap_range(curproc->p_pagedir, USER_MEM_LOW, USER_MEM_HIGH); 
        tlb_flush_all();

        sched_make_runnable(child_thread);
        regs->r_eax = child_proc->p_pid;
        dbg(DBG_PRINT, "(GRADING3A)\n"); 
        return child_proc->p_pid;        
        // NOT_YET_IMPLEMENTED("VM: do_fork");
        // return 0;
}
