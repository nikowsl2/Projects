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

/*
 *  FILE: open.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Mon Apr  6 19:27:49 1998
 */

#include "globals.h"
#include "errno.h"
#include "fs/fcntl.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/stat.h"
#include "util/debug.h"

/* find empty index in p->p_files[] */
int
get_empty_fd(proc_t *p)
{
        int fd;

        for (fd = 0; fd < NFILES; fd++) {
                if (!p->p_files[fd])
                        return fd;
        }

        dbg(DBG_ERROR | DBG_VFS, "ERROR: get_empty_fd: out of file descriptors "
            "for pid %d\n", curproc->p_pid);
        return -EMFILE;
}

/*
 * There a number of steps to opening a file:
 *      1. Get the next empty file descriptor.
 *      2. Call fget to get a fresh file_t.
 *      3. Save the file_t in curproc's file descriptor table.
 *      4. Set file_t->f_mode to OR of FMODE_(READ|WRITE|APPEND) based on
 *         oflags, which can be O_RDONLY, O_WRONLY or O_RDWR, possibly OR'd with
 *         O_APPEND.
 *      5. Use open_namev() to get the vnode for the file_t.
 *      6. Fill in the fields of the file_t.
 *      7. Return new fd.
 *
 * If anything goes wrong at any point (specifically if the call to open_namev
 * fails), be sure to remove the fd from curproc, fput the file_t and return an
 * error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        oflags is not valid.
 *      o EMFILE
 *        The process already has the maximum number of files open.
 *      o ENOMEM
 *        Insufficient kernel memory was available.
 *      o ENAMETOOLONG
 *        A component of filename was too long.
 *      o ENOENT
 *        O_CREAT is not set and the named file does not exist.  Or, a
 *        directory component in pathname does not exist.
 *      o EISDIR
 *        pathname refers to a directory and the access requested involved
 *        writing (that is, O_WRONLY or O_RDWR is set).
 *      o ENXIO
 *        pathname refers to a device special file and no corresponding device
 *        exists.
 */

int
do_open(const char *filename, int oflags)
{
        if(strlen(filename) > NAME_LEN) {
                
                return -ENAMETOOLONG;
        }

        if ((oflags & O_WRONLY) && (oflags & O_RDWR)) {
                
                return -EINVAL;
        }

        file_t *file;
        vnode_t *vnode;
        int fd;
  
        fd = get_empty_fd(curproc);
        // if (fd < 0) {
        //         dbg(DBG_PRINT, "(GRADING2C? 3)\n");
        //         return fd;
        // }

        file = fget(-1);
        // if (!file) {
        //         dbg(DBG_PRINT, "(GRADING2C? 4)\n");
        //         return -ENOMEM;
        // }

        KASSERT(!curproc->p_files[fd]);
        curproc->p_files[fd] = file;
        
        int access_flags = oflags & ~O_CREAT;  
        
        if (access_flags == O_RDONLY) {
                file->f_mode = FMODE_READ;
              
        } 
        else if (access_flags == O_WRONLY) {
                file->f_mode = FMODE_WRITE;
                
        } 
        else if (access_flags == O_RDWR) {
                file->f_mode = FMODE_READ | FMODE_WRITE;
                
        } 
        // else if (access_flags == (O_RDONLY | O_APPEND)) {
        //         file->f_mode = FMODE_READ | FMODE_APPEND;
        // } 

        // else if (access_flags == (O_WRONLY | O_APPEND)) {
        //         file->f_mode = FMODE_WRITE | FMODE_APPEND;
        // } 

        else if (access_flags == (O_RDWR | O_APPEND)) {
                file->f_mode = FMODE_READ | FMODE_WRITE | FMODE_APPEND;
                
        } 

        // else {
               
        //         do_close(fd);                
        //         return -EINVAL;
        // }
        
        int result = open_namev(filename, oflags, &vnode, NULL);
        if (result < 0) {               
                do_close(fd);
                
                return result;
        }
        if (S_ISDIR(vnode->vn_mode) && (oflags & O_WRONLY || oflags & O_RDWR) )
        {                
                vput(vnode);
                do_close(fd);
                
                return -EISDIR;
        }
  
        // if(S_ISCHR(vnode->vn_mode))
        // {
        //         if(!(vnode->vn_cdev = bytedev_lookup(vnode->vn_devid)))
        //         {
        //                 vput(vnode);
        //                 do_close(fd);
        //                 
        //                 return -ENXIO;
        //         }
        //         dbg(DBG_PRINT, "(GRADING2B? 15)\n");
        // }
        // if(S_ISBLK(vnode->vn_mode))
        // {
        //         if(!(vnode->vn_bdev = blockdev_lookup(vnode->vn_devid)))
        //         {
        //                 vput(vnode);
        //                 do_close(fd);
        //                 dbg(DBG_PRINT, "(GRADING2C? 16)\n");
        //                 return -ENXIO;
        //         }
        //         dbg(DBG_PRINT, "(GRADING2C? 17)\n");
        // }

        file->f_vnode = vnode;
        file->f_pos = 0;
        
        return fd;
        //NOT_YET_IMPLEMENTED("VFS: do_open");
        //return -1;
}
