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
#include "kernel.h"
#include "errno.h"

#include "util/gdb.h"
#include "util/init.h"
#include "util/debug.h"
#include "util/string.h"
#include "util/printf.h"

#include "mm/mm.h"
#include "mm/page.h"
#include "mm/pagetable.h"
#include "mm/pframe.h"

#include "vm/vmmap.h"
#include "vm/shadowd.h"
#include "vm/shadow.h"
#include "vm/anon.h"

#include "main/acpi.h"
#include "main/apic.h"
#include "main/interrupt.h"
#include "main/gdt.h"

#include "proc/sched.h"
#include "proc/proc.h"
#include "proc/kthread.h"

#include "drivers/dev.h"
#include "drivers/blockdev.h"
#include "drivers/disk/ata.h"
#include "drivers/tty/virtterm.h"
#include "drivers/pci.h"

#include "api/exec.h"
#include "api/syscall.h"

#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/fcntl.h"
#include "fs/stat.h"

#include "test/kshell/kshell.h"
#include "test/s5fs_test.h"


GDB_DEFINE_HOOK(initialized)

void      *bootstrap(int arg1, void *arg2);
void      *idleproc_run(int arg1, void *arg2);
kthread_t *initproc_create(void);
void      *initproc_run(int arg1, void *arg2);
void      *final_shutdown(void);
int do_faber_thread_test(kshell_t *kshell, int argc, char **argv);
int do_sunghan_test(kshell_t *kshell, int argc, char **argv);
int do_sunghan_deadlock_test(kshell_t *kshell, int argc, char **argv);
int do_vfs_test(kshell_t *kshell, int argc, char **argv);
int do_usr_hello(kshell_t *kshell, int argc, char **argv);
extern void *faber_thread_test(int, void*);
extern void *sunghan_test(int, void*);
extern void *sunghan_deadlock_test(int, void*);
extern void *vfstest_main(int, void*);

#ifdef __VM__
void usrland_test(void);
static void *vm_test(int arg1, void *arg2);  
static void vm_test_run(void);
#endif

extern int faber_fs_thread_test(kshell_t *ksh, int argc, char **argv);
extern int faber_directory_test(kshell_t *ksh, int argc, char **argv);

static struct stat root_stat;
static struct stat null_stat;
static struct stat zero_stat;
static struct stat tty0_stat;
typedef struct {
    struct proc *p;
    struct kthread *t;
} proc_thread_t;

/**
 * This function is called from kmain, however it is not running in a
 * thread context yet. It should create the idle process which will
 * start executing idleproc_run() in a real thread context.  To start
 * executing in the new process's context call context_make_active(),
 * passing in the appropriate context. This function should _NOT_
 * return.
 *
 * Note: Don't forget to set curproc and curthr appropriately.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
void *
bootstrap(int arg1, void *arg2)
{
        /* If the next line is removed/altered in your submission, 20 points will be deducted. */
        dbgq(DBG_TEST, "SIGNATURE: 53616c7465645f5f51fd0a9c3c6ab82304a111643ec02df566e88e98974094e4c7710eb2ba2432d8f18b263b4204815c\n");
        /* necessary to finalize page table information */
        pt_template_init();

        proc_t *idle = proc_create("idle");
        KASSERT(idle != NULL);
        KASSERT(idle->p_pid == PID_IDLE);
        kthread_t *idle_thread = kthread_create(idle, idleproc_run, 0, NULL);
        KASSERT(idle_thread != NULL);

        curproc = idle;
        KASSERT(NULL != curproc);
        curthr = idle_thread;
        KASSERT(NULL != curthr);
        //dbg(DBG_PRINT, "(GRADING1A)\n");
        context_make_active(&curthr->kt_ctx);

        //NOT_YET_IMPLEMENTED("PROCS: bootstrap");

        panic("weenix returned to bootstrap()!!! BAD!!!\n");
        return NULL;
}

/**
 * Once we're inside of idleproc_run(), we are executing in the context of the
 * first process-- a real context, so we can finally begin running
 * meaningful code.
 *
 * This is the body of process 0. It should initialize all that we didn't
 * already initialize in kmain(), launch the init process (initproc_run),
 * wait for the init process to exit, then halt the machine.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
void *
idleproc_run(int arg1, void *arg2)
{
        int status;
        pid_t child;

        /* create init proc */
        kthread_t *initthr = initproc_create();
        init_call_all();
        GDB_CALL_HOOK(initialized);

        /* Create other kernel threads (in order) */

#ifdef __VFS__
        /* Once you have VFS remember to set the current working directory
         * of the idle and init processes */
        //NOT_YET_IMPLEMENTED("VFS: idleproc_run");
        curproc->p_cwd = vfs_root_vn;
        vref(vfs_root_vn);
        
        initthr->kt_proc->p_cwd = vfs_root_vn;
        vref(vfs_root_vn);                

        /* Here you need to make the null, zero, and tty devices using mknod */
        /* You can't do this until you have VFS, check the include/drivers/dev.h
         * file for macros with the device ID's you will need to pass to mknod */
        //NOT_YET_IMPLEMENTED("VFS: idleproc_run");
        int do_mkdir_result = do_mkdir("/dev");
        KASSERT(do_mkdir_result == 0);
        int null_mknod_result = do_mknod("/dev/null", S_IFCHR, MKDEVID(1,0));
        KASSERT(null_mknod_result == 0);
        // do_stat("/dev/null", &null_stat);
      
        int zero_mknod_result = do_mknod("/dev/zero", S_IFCHR, MKDEVID(1,1));
        KASSERT(zero_mknod_result == 0);
        // do_stat("/dev/zero", &zero_stat);
  
        int tty0_mknod_result = do_mknod("/dev/tty0", S_IFCHR, MKDEVID(2, 0));
        KASSERT(tty0_mknod_result == 0);
        
        // do_stat("/dev/tty0", &tty0_stat);
        
       
#endif

        /* Finally, enable interrupts (we want to make sure interrupts
         * are enabled AFTER all drivers are initialized) */
        intr_enable();

        /* Run initproc */
        sched_make_runnable(initthr);
        /* Now wait for it */
        child = do_waitpid(-1, 0, &status);
        KASSERT(PID_INIT == child);
        //dbg(DBG_PRINT, "(GRADING1A)\n");
        return final_shutdown();
}

/**
 * This function, called by the idle process (within 'idleproc_run'), creates the
 * process commonly refered to as the "init" process, which should have PID 1.
 *
 * The init process should contain a thread which begins execution in
 * initproc_run().
 *
 * @return a pointer to a newly created thread which will execute
 * initproc_run when it begins executing
 */
kthread_t *
initproc_create(void)
{
        proc_t *init = proc_create("init");
        KASSERT(init != NULL);
        KASSERT(init->p_pid == PID_INIT);
        kthread_t *init_thread = kthread_create(init, initproc_run, 0, NULL);
        KASSERT(init_thread != NULL);
        //dbg(DBG_PRINT, "(GRADING1A)\n");

        //NOT_YET_IMPLEMENTED("PROCS: initproc_create");
        return init_thread;
}


/**
 * The init thread's function changes depending on how far along your Weenix is
 * developed. Before VM/FI, you'll probably just want to have this run whatever
 * tests you've written (possibly in a new process). After VM/FI, you'll just
 * exec "/sbin/init".
 *
 * Both arguments are unused.
 *
 * @param arg1 the first argument (unused)
 * @param arg2 the second argument (unused)
 */
void *
initproc_run(int arg1, void *arg2)
{
        //NOT_YET_IMPLEMENTED("PROCS: initproc_run");
        //faber_thread_test(0, NULL);
       
        //kshell_add_command("faber", do_faber_thread_test, "invoke do_faber_thread_test()...");
        //kshell_add_command("sunghan", do_sunghan_test, "invoke do_sunghan_test()...");
        //kshell_add_command("deadlock", do_sunghan_deadlock_test, "invoke do_sunghan_deadlock_test()...");
        // kshell_add_command("vfstest", do_vfs_test, "invoke do_vfs_test()...");
        //kshell_add_command("thrtest", faber_fs_thread_test, "Run faber_fs_thread_test().");
        // kshell_add_command("dirtest", faber_directory_test, "Run faber_directory_test().");
        
        // int err = 0;
        // kshell_t *kshell = kshell_create(0);
        // KASSERT(kshell != NULL && "Couldn't create kernel shell");        
        // while ((err = kshell_execute_next(kshell)) > 0);
        // KASSERT(err == 0 && "kernel shell exited with an error\n");
        // kshell_destroy(kshell);
        // char *argsvec[] = {NULL};
        // char *empty_env[] = {NULL};       
        // kernel_execve("/usr/bin/hello", argsvec, empty_env);
        // dbg(DBG_PRINT, "(kmain ending?????????)\n");
        
        // int fd = do_open("/dev/tty0", O_RDWR); 
        // KASSERT(fd >= 0 && "Failed to open /dev/tty0");

        
        // do_dup2(fd, 1); 
        // do_dup2(fd, 2); 
        // char *argsvec2[] = {NULL}; 
        // char *empty_env2[] = {NULL}; 

        // kernel_execve("/usr/bin/fork-and-wait", argsvec2, empty_env2);
        // char *argv[] = { NULL };
        // char *envp[] = { NULL };

        // kernel_execve("/sbin/init", argv, envp);
        char *argv[] = { NULL };
        char *envp[] = { NULL };
        kernel_execve("/sbin/init", argv, envp);
        
        
        
        return NULL;
}



#ifdef __VFS__
int do_vfs_test(kshell_t *kshell, int argc, char **argv)
{                
        KASSERT(kshell != NULL);
        int status;
        proc_t* vfs_test_process = proc_create("vfs");
        kthread_t* vfs_test_thread = kthread_create(vfs_test_process, vfstest_main, 1, NULL);
        sched_make_runnable(vfs_test_thread);
        int pid = do_waitpid(vfs_test_process->p_pid, 0, &status);      
                
        return 0;
}
#endif

#ifdef __DRIVERS__
int do_faber_thread_test(kshell_t *kshell, int argc, char **argv)
{                
        KASSERT(kshell != NULL);
        int status;
        proc_t* faber_test_process = proc_create("faber");
        kthread_t* faber_test_thread = kthread_create(faber_test_process, faber_thread_test, 0, NULL);
        sched_make_runnable(faber_test_thread);
        int pid = do_waitpid(faber_test_process->p_pid, 0, &status);      
                
        return 0;
}

int do_sunghan_test(kshell_t *kshell, int argc, char **argv)
{                
        KASSERT(kshell != NULL);
        int status;
        proc_t* sunghan_test_process = proc_create("sunghan");
        kthread_t* sunghan_test_thread = kthread_create(sunghan_test_process, sunghan_test, 0, NULL);
        sched_make_runnable(sunghan_test_thread);
        do_waitpid(sunghan_test_process->p_pid, 0, &status);              

        return 0;
}

int do_sunghan_deadlock_test(kshell_t *kshell, int argc, char **argv)
{                
        KASSERT(kshell != NULL);
        int status;
        proc_t* sunghan_deadlock_test_process = proc_create("sunghan_deadlock");
        kthread_t* sunghan_deadlock_test_thread = kthread_create(sunghan_deadlock_test_process, sunghan_deadlock_test, 0, NULL);
        sched_make_runnable(sunghan_deadlock_test_thread);
        do_waitpid(sunghan_deadlock_test_process->p_pid, 0, &status);      

        return 0;
}
#endif /* __DRIVERS__ */