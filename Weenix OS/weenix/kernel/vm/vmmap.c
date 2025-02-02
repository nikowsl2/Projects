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

#include "kernel.h"
#include "errno.h"
#include "globals.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/anon.h"

#include "proc/proc.h"

#include "util/debug.h"
#include "util/list.h"
#include "util/string.h"
#include "util/printf.h"

#include "fs/vnode.h"
#include "fs/file.h"
#include "fs/fcntl.h"
#include "fs/vfs_syscall.h"

#include "mm/slab.h"
#include "mm/page.h"
#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/mmobj.h"
#include "mm/tlb.h"

static slab_allocator_t *vmmap_allocator;
static slab_allocator_t *vmarea_allocator;

void
vmmap_init(void)
{
        vmmap_allocator = slab_allocator_create("vmmap", sizeof(vmmap_t));
        KASSERT(NULL != vmmap_allocator && "failed to create vmmap allocator!");
        vmarea_allocator = slab_allocator_create("vmarea", sizeof(vmarea_t));
        KASSERT(NULL != vmarea_allocator && "failed to create vmarea allocator!");
}

vmarea_t *
vmarea_alloc(void)
{
        vmarea_t *newvma = (vmarea_t *) slab_obj_alloc(vmarea_allocator);
        if (newvma) {
                newvma->vma_vmmap = NULL;
        }
        return newvma;
}

void
vmarea_free(vmarea_t *vma)
{
        KASSERT(NULL != vma);
        slab_obj_free(vmarea_allocator, vma);
}

/* a debugging routine: dumps the mappings of the given address space. */
size_t
vmmap_mapping_info(const void *vmmap, char *buf, size_t osize)
{
        KASSERT(0 < osize);
        KASSERT(NULL != buf);
        KASSERT(NULL != vmmap);

        vmmap_t *map = (vmmap_t *)vmmap;
        vmarea_t *vma;
        ssize_t size = (ssize_t)osize;

        int len = snprintf(buf, size, "%21s %5s %7s %8s %10s %12s\n",
                           "VADDR RANGE", "PROT", "FLAGS", "MMOBJ", "OFFSET",
                           "VFN RANGE");

        list_iterate_begin(&map->vmm_list, vma, vmarea_t, vma_plink) {
                size -= len;
                buf += len;
                if (0 >= size) {
                        goto end;
                }

                len = snprintf(buf, size,
                               "%#.8x-%#.8x  %c%c%c  %7s 0x%p %#.5x %#.5x-%#.5x\n",
                               vma->vma_start << PAGE_SHIFT,
                               vma->vma_end << PAGE_SHIFT,
                               (vma->vma_prot & PROT_READ ? 'r' : '-'),
                               (vma->vma_prot & PROT_WRITE ? 'w' : '-'),
                               (vma->vma_prot & PROT_EXEC ? 'x' : '-'),
                               (vma->vma_flags & MAP_SHARED ? " SHARED" : "PRIVATE"),
                               vma->vma_obj, vma->vma_off, vma->vma_start, vma->vma_end);
        } list_iterate_end();

end:
        if (size <= 0) {
                size = osize;
                buf[osize - 1] = '\0';
        }
        /*
        KASSERT(0 <= size);
        if (0 == size) {
                size++;
                buf--;
                buf[0] = '\0';
        }
        */
        return osize - size;
}

/* Create a new vmmap, which has no vmareas and does
 * not refer to a process. */
vmmap_t *
vmmap_create(void)
{
        vmmap_t *new_vmmap = (vmmap_t *) slab_obj_alloc(vmmap_allocator);
       
        new_vmmap->vmm_proc = NULL;
        list_init(&new_vmmap->vmm_list);
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return new_vmmap;
        // NOT_YET_IMPLEMENTED("VM: vmmap_create");
        // return NULL;
}

/* Removes all vmareas from the address space and frees the
 * vmmap struct. */
void
vmmap_destroy(vmmap_t *map)
{
        KASSERT(NULL != map);
        dbg(DBG_PRINT, "(GRADING3A 3.a)\n");

        vmarea_t *current_area;        
        list_iterate_begin( &map->vmm_list, current_area, vmarea_t, vma_plink ) {
            
            if (list_link_is_linked(&current_area->vma_olink)){
                list_remove(&current_area->vma_olink);
                dbg(DBG_PRINT, "(GRADING3A)\n");
            }
            if (list_link_is_linked(&current_area->vma_plink)){
                list_remove(&current_area->vma_plink);
                dbg(DBG_PRINT, "(GRADING3A)\n");
            }

            current_area->vma_obj->mmo_ops->put(current_area->vma_obj);            
            vmarea_free(current_area);       
            dbg(DBG_PRINT, "(GRADING3A)\n");
        } 
        list_iterate_end();
 
        slab_obj_free(vmmap_allocator, map);
        dbg(DBG_PRINT, "(GRADING3A)\n");
        //NOT_YET_IMPLEMENTED("VM: vmmap_destroy");
}

/* Add a vmarea to an address space. Assumes (i.e. asserts to some extent)
 * the vmarea is valid.  This involves finding where to put it in the list
 * of VM areas, and adding it. Don't forget to set the vma_vmmap for the
 * area. */
void
vmmap_insert(vmmap_t *map, vmarea_t *newvma)
{
        KASSERT(NULL != map && NULL != newvma);
        KASSERT(NULL == newvma->vma_vmmap);
        KASSERT(newvma->vma_start < newvma->vma_end);
        KASSERT(ADDR_TO_PN(USER_MEM_LOW) <= newvma->vma_start && ADDR_TO_PN(USER_MEM_HIGH) >= newvma->vma_end);
        dbg(DBG_PRINT, "(GRADING3A 3.b)\n");
        newvma->vma_vmmap = map;

        if(list_empty(&map->vmm_list)){
                list_insert_tail(&map->vmm_list, &newvma->vma_plink);
                dbg(DBG_PRINT, "(GRADING3A)\n");
                return;
        }

        vmarea_t *current_area;
        list_iterate_begin(&map->vmm_list, current_area, vmarea_t, vma_plink) {
                if (newvma->vma_start < current_area->vma_start) {
                        list_insert_before(&current_area->vma_plink, &newvma->vma_plink);
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                        return;
                }
        } 
        
        list_iterate_end();
        list_insert_tail(&map->vmm_list, &newvma->vma_plink);
        dbg(DBG_PRINT, "(GRADING3A)\n");
        //NOT_YET_IMPLEMENTED("VM: vmmap_insert");
}

/* Find a contiguous range of free virtual pages of length npages in
 * the given address space. Returns starting vfn for the range,
 * without altering the map. Returns -1 if no such range exists.
 *
 * Your algorithm should be first fit. If dir is VMMAP_DIR_HILO, you
 * should find a gap as high in the address space as possible; if dir
 * is VMMAP_DIR_LOHI, the gap should be as low as possible. */
int
vmmap_find_range(vmmap_t *map, uint32_t npages, int dir)
{
        KASSERT(NULL != map);
        
        // if (list_empty(&map->vmm_list)) {
        //         if (dir == VMMAP_DIR_HILO) {
        //                 dbg(DBG_PRINT, "(GRADING3? find1)\n");
        //                 return (USER_MEM_HIGH / PAGE_SIZE) - npages;
        //         } 
        //         else {
        //                 dbg(DBG_PRINT, "(GRADING3? find2)\n");
        //                 return USER_MEM_LOW / PAGE_SIZE;
        //         }
        // }

        if (dir == VMMAP_DIR_HILO) {
             
                vmarea_t *last = list_tail(&map->vmm_list, vmarea_t, vma_plink);
                uint32_t high = USER_MEM_HIGH / PAGE_SIZE;
                if (high - last->vma_end >= npages) {
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                        return high - npages;
                }

               
                vmarea_t *curr;
                list_iterate_reverse(&map->vmm_list, curr, vmarea_t, vma_plink) {
                        if (curr->vma_plink.l_prev != &map->vmm_list) {
                                vmarea_t *vma_prev = list_item(curr->vma_plink.l_prev, vmarea_t, vma_plink);
                                if (curr->vma_start - vma_prev->vma_end >= npages) {
                                        dbg(DBG_PRINT, "(GRADING3D 1)\n");
                                        return curr->vma_start - npages;
                                }
                                dbg(DBG_PRINT, "(GRADING3D 1)\n");
                        }
                        else {
                                uint32_t low = USER_MEM_LOW / PAGE_SIZE;
                                if (curr->vma_start - low >= npages) {
                                        dbg(DBG_PRINT, "(GRADING3D 2)\n");
                                        return curr->vma_start - npages;
                                }
                                dbg(DBG_PRINT, "(GRADING3D 2)\n");
                        }
                } 
                list_iterate_end();
        }
        // else{
        //         vmarea_t *first = list_head(&map->vmm_list, vmarea_t, vma_plink);
        //         uint32_t low = USER_MEM_LOW / PAGE_SIZE;
        //         if (first->vma_start - low >= npages) {
        //                 dbg(DBG_PRINT, "(GRADING3? find8)\n");
        //                 return low;
        //         }

        //         vmarea_t *curr;
        //         list_iterate_begin(&map->vmm_list, curr, vmarea_t, vma_plink) {
        //                 if (curr->vma_plink.l_next != &map->vmm_list) {
        //                         vmarea_t *vma_next = list_item(curr->vma_plink.l_next, vmarea_t, vma_plink);
        //                         if (vma_next->vma_start - curr->vma_end >= npages) {
        //                                 dbg(DBG_PRINT, "(GRADING3? find9)\n");
        //                                 return curr->vma_end;
        //                         }
        //                         dbg(DBG_PRINT, "(GRADING3? find10)\n");
        //                 }
        //                 else {
        //                         uint32_t high = USER_MEM_HIGH / PAGE_SIZE;
        //                         if (high - curr->vma_end >= npages) {
        //                                 dbg(DBG_PRINT, "(GRADING3? find11)\n");
        //                                 return curr->vma_end;
        //                         }
        //                         dbg(DBG_PRINT, "(GRADING3? find12)\n");
        //                 }
        //         } 
        //         list_iterate_end();
        // }
        dbg(DBG_PRINT, "(GRADING3D 2)\n");
        //NOT_YET_IMPLEMENTED("VM: vmmap_find_range");
        return -1;
}

/* Find the vm_area that vfn lies in. Simply scan the address space
 * looking for a vma whose range covers vfn. If the page is unmapped,
 * return NULL. */
vmarea_t *
vmmap_lookup(vmmap_t *map, uint32_t vfn)
{
        KASSERT(NULL != map);
        dbg(DBG_PRINT, "(GRADING3A 3.c)\n");

        vmarea_t *curr;
        list_iterate_begin(&map->vmm_list, curr, vmarea_t, vma_plink) {
                if (curr->vma_start <= vfn && curr->vma_end > vfn) {
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                        return curr;
                }
        } 
        list_iterate_end();
        dbg(DBG_PRINT, "(GRADING3D 1)\n");

        return NULL;

        //NOT_YET_IMPLEMENTED("VM: vmmap_lookup");
        //return NULL;
}

/* Allocates a new vmmap containing a new vmarea for each area in the
 * given map. The areas should have no mmobjs set yet. Returns pointer
 * to the new vmmap on success, NULL on failure. This function is
 * called when implementing fork(2). */
vmmap_t *
vmmap_clone(vmmap_t *map)
{
        vmmap_t *new_vmmap = vmmap_create();
        // if (NULL == new_vmmap) {
        //         dbg(DBG_PRINT, "(GRADING3? clone1)\n");
        //         return NULL;
        // }
        vmarea_t *old_vma;
        list_iterate_begin(&map->vmm_list, old_vma, vmarea_t, vma_plink) {
                vmarea_t *new_vma = vmarea_alloc();
                new_vma->vma_start = old_vma->vma_start;
                new_vma->vma_end = old_vma->vma_end;
                new_vma->vma_prot = old_vma->vma_prot;
                new_vma->vma_flags = old_vma->vma_flags;
                new_vma->vma_off = old_vma->vma_off;
                list_link_init(&new_vma->vma_olink);                
                list_link_init(&new_vma->vma_plink);
                vmmap_insert(new_vmmap,new_vma);                
                dbg(DBG_PRINT, "(GRADING3A)\n");
        } 
        list_iterate_end();
        dbg(DBG_PRINT, "(GRADING3A)\n");

        return new_vmmap;
        // NOT_YET_IMPLEMENTED("VM: vmmap_clone");
        // return NULL;
}

/* Insert a mapping into the map starting at lopage for npages pages.
 * If lopage is zero, we will find a range of virtual addresses in the
 * process that is big enough, by using vmmap_find_range with the same
 * dir argument.  If lopage is non-zero and the specified region
 * contains another mapping that mapping should be unmapped.
 *
 * If file is NULL an anon mmobj will be used to create a mapping
 * of 0's.  If file is non-null that vnode's file will be mapped in
 * for the given range.  Use the vnode's mmap operation to get the
 * mmobj for the file; do not assume it is file->vn_obj. Make sure all
 * of the area's fields except for vma_obj have been set before
 * calling mmap.
 *
 * If MAP_PRIVATE is specified set up a shadow object for the mmobj.
 *
 * All of the input to this function should be valid (KASSERT!).
 * See mmap(2) for for description of legal input.
 * Note that off should be page aligned.
 *
 * Be very careful about the order operations are performed in here. Some
 * operation are impossible to undo and should be saved until there
 * is no chance of failure.
 *
 * If 'new' is non-NULL a pointer to the new vmarea_t should be stored in it.
 */
int
vmmap_map(vmmap_t *map, vnode_t *file, uint32_t lopage, uint32_t npages,
          int prot, int flags, off_t off, int dir, vmarea_t **new)
{
        KASSERT(NULL != map);
        KASSERT(0 < npages);
        KASSERT((MAP_SHARED & flags) || (MAP_PRIVATE & flags));
        KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_LOW) <= lopage));
        KASSERT((0 == lopage) || (ADDR_TO_PN(USER_MEM_HIGH) >= (lopage + npages)));
        KASSERT(PAGE_ALIGNED(off));
        dbg(DBG_PRINT, "(GRADING3A 3.d)\n");
        
        if(lopage == 0){
                int find_result =  vmmap_find_range(map, npages, dir);
                if(find_result == -1){
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                        return -1;
                }
                lopage = (uint32_t)find_result;
                dbg(DBG_PRINT, "(GRADING3A)\n");
        }
        else{
                int check_range = vmmap_is_range_empty(map, lopage, npages);
                if(check_range == 0){
                        vmmap_remove(map, lopage, npages);
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                }
                dbg(DBG_PRINT, "(GRADING3A)\n");
        }
        vmarea_t *new_vmarea = vmarea_alloc();
        mmobj_t *new_mmobj;
        
        new_vmarea->vma_start = lopage;
        new_vmarea->vma_flags = flags;
        new_vmarea->vma_prot = prot;
        new_vmarea->vma_off = ADDR_TO_PN(off);
        new_vmarea->vma_end = lopage + npages;

        list_link_init(&new_vmarea->vma_plink);
        list_link_init(&new_vmarea->vma_olink);

        if(file == NULL){
                new_mmobj = anon_create(); 
                dbg(DBG_PRINT, "(GRADING3A)\n");
        }

        else{
                file->vn_ops->mmap(file,new_vmarea,&new_mmobj);        
                dbg(DBG_PRINT, "(GRADING3A)\n");
        }

        if(flags & MAP_PRIVATE){
                mmobj_t *shadow_mmobj = shadow_create();
                shadow_mmobj->mmo_shadowed = new_mmobj;
                
                mmobj_t *bottom_obj = mmobj_bottom_obj(new_mmobj);
                shadow_mmobj->mmo_un.mmo_bottom_obj = bottom_obj;
                new_vmarea->vma_obj = shadow_mmobj;  
            
                list_insert_head(&(bottom_obj->mmo_un.mmo_vmas), &(new_vmarea->vma_olink));  
                dbg(DBG_PRINT, "(GRADING3A)\n");
        }

        else{
                new_vmarea->vma_obj = new_mmobj;
                list_insert_head(&(new_mmobj->mmo_un.mmo_vmas), &(new_vmarea->vma_olink));
                dbg(DBG_PRINT, "(GRADING3D 1)\n");
        }

        vmmap_insert(map, new_vmarea);
        if(new != NULL){
                *new = new_vmarea;
                dbg(DBG_PRINT, "(GRADING3A)\n");
        }
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 0;

        // NOT_YET_IMPLEMENTED("VM: vmmap_map");
        // return -1;
}

/*
 * We have no guarantee that the region of the address space being
 * unmapped will play nicely with our list of vmareas.
 *
 * You must iterate over each vmarea that is partially or wholly covered
 * by the address range [addr ... addr+len). The vm-area will fall into one
 * of four cases, as illustrated below:
 *
 * key:
 *          [             ]   Existing VM Area
 *        *******             Region to be unmapped
 *
 * Case 1:  [   ******    ]
 * The region to be unmapped lies completely inside the vmarea. We need to
 * split the old vmarea into two vmareas. be sure to increment the
 * reference count to the file associated with the vmarea.
 *
 * Case 2:  [      *******]**
 * The region overlaps the end of the vmarea. Just shorten the length of
 * the mapping.
 *
 * Case 3: *[*****        ]
 * The region overlaps the beginning of the vmarea. Move the beginning of
 * the mapping (remember to update vma_off), and shorten its length.
 *
 * Case 4: *[*************]**
 * The region completely contains the vmarea. Remove the vmarea from the
 * list.
 */
int
vmmap_remove(vmmap_t *map, uint32_t lopage, uint32_t npages)
{
        KASSERT(NULL != map);
        
        if (npages == 0 || vmmap_is_range_empty(map, lopage, npages)) {
                dbg(DBG_PRINT, "(GRADING3D 2)\n");
                return 0;
        }
        
        uint32_t hipage = lopage + npages;
        vmarea_t *curr;        

        list_iterate_begin(&map->vmm_list, curr, vmarea_t, vma_plink) {
                if (curr->vma_end <= lopage || curr->vma_start >= hipage) {
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                        continue;
                }
                
                if (lopage > curr->vma_start && hipage < curr->vma_end) {
                        vmarea_t *new_vma = vmarea_alloc();
                        // if (new_vma == NULL) {
                        //         dbg(DBG_PRINT, "(GRADING3? vremove3)\n");
                        //         return -ENOMEM;
                        // }
                        
                        new_vma->vma_start = curr->vma_start;
                        new_vma->vma_end = lopage;            
                        new_vma->vma_off = curr->vma_off;     
                        new_vma->vma_prot = curr->vma_prot;   
                        new_vma->vma_flags = curr->vma_flags; 
                        new_vma->vma_obj = curr->vma_obj;     
                        new_vma->vma_vmmap = map;
                        curr->vma_obj->mmo_ops->ref(curr->vma_obj); 
                        
                        list_link_init(&new_vma->vma_olink);
                        list_link_init(&new_vma->vma_plink);
                        list_insert_before(&curr->vma_plink, &new_vma->vma_plink);
                        
                        mmobj_t *bottom = mmobj_bottom_obj(curr->vma_obj);
                        list_insert_head(&bottom->mmo_un.mmo_vmas, &new_vma->vma_olink);
          
                        curr->vma_off += (hipage - curr->vma_start);  
                        curr->vma_start = hipage;
                        dbg(DBG_PRINT, "(GRADING3D 2)\n");
                }

                else if (curr->vma_start < lopage) {
                        curr->vma_end = lopage;
                        dbg(DBG_PRINT, "(GRADING3D 1)\n");
                }

                else if (curr->vma_end > hipage) {
                        curr->vma_off += (hipage - curr->vma_start);
                        curr->vma_start = hipage;
                        dbg(DBG_PRINT, "(GRADING3D 2)\n");
                }
                
                else {
                        list_remove(&curr->vma_plink);
                        if (list_link_is_linked(&curr->vma_olink)) {
                                list_remove(&curr->vma_olink);
                                dbg(DBG_PRINT, "(GRADING3A)\n");
                        }
                        curr->vma_obj->mmo_ops->put(curr->vma_obj);
                        vmarea_free(curr);
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                }
        } 
        list_iterate_end();

        tlb_flush_range((uintptr_t)PN_TO_ADDR(lopage), npages);
        pt_unmap_range(curproc->p_pagedir, (uintptr_t)PN_TO_ADDR(lopage),
                      (uintptr_t)PN_TO_ADDR(lopage + npages));

        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 0;
        // NOT_YET_IMPLEMENTED("VM: vmmap_remove");
        // return -1;
}

/*
 * Returns 1 if the given address space has no mappings for the
 * given range, 0 otherwise.
 */
int
vmmap_is_range_empty(vmmap_t *map, uint32_t startvfn, uint32_t npages)
{
        KASSERT(NULL != map);
        
        uint32_t endvfn = startvfn + npages;
        KASSERT((startvfn < endvfn) && (ADDR_TO_PN(USER_MEM_LOW) <= startvfn) && (ADDR_TO_PN(USER_MEM_HIGH) >= endvfn));
        dbg(DBG_PRINT, "(GRADING3A 3.e)\n");
        
        vmarea_t *curr_vmarea;

        list_iterate_begin(&map->vmm_list, curr_vmarea, vmarea_t, vma_plink) {
             
                if (curr_vmarea->vma_start < endvfn && curr_vmarea->vma_end > startvfn) {
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                        return 0;
                }
        } 
        list_iterate_end();

        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 1;
        //NOT_YET_IMPLEMENTED("VM: vmmap_is_range_empty");
        //return 0;
}

/* Read into 'buf' from the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do so, you will want to find the vmareas
 * to read from, then find the pframes within those vmareas corresponding
 * to the virtual addresses you want to read, and then read from the
 * physical memory that pframe points to. You should not check permissions
 * of the areas. Assume (KASSERT) that all the areas you are accessing exist.
 * Returns 0 on success, -errno on error.
 */
int
vmmap_read(vmmap_t *map, const void *vaddr, void *buf, size_t count)
{
        // KASSERT(NULL != map);
        // KASSERT(NULL != vaddr);
        // KASSERT(NULL != buf);
        // KASSERT(count > 0);

        uint32_t buf_index = (uint32_t)buf;
        uint32_t addr = (uint32_t)vaddr;
        uint32_t bytes_remaining = count;

        while (bytes_remaining > 0) {
                
                vmarea_t *vma = vmmap_lookup(map, ADDR_TO_PN(addr));
                // if (vma == NULL) {
                //         dbg(DBG_PRINT, "(GRADING3? vread1)\n");
                //         return -EFAULT;
                // }
                KASSERT(vma);

                uint32_t page_num = ADDR_TO_PN(addr) + vma->vma_off - vma->vma_start;
        
                pframe_t *pframe;
                int err = pframe_lookup(vma->vma_obj, page_num, 0, &pframe);
                // if (err < 0) {
                //         dbg(DBG_PRINT, "(GRADING3? vread2)\n");
                //         return err;
                // }

                uint32_t page_offset = PAGE_OFFSET(addr);
                uint32_t bytes_this_page = PAGE_SIZE - page_offset;
                if (bytes_this_page > bytes_remaining) {
                        bytes_this_page = bytes_remaining;
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                }

               
                memcpy((void *)buf_index, 
                      (char *)pframe->pf_addr + page_offset, 
                      bytes_this_page);

                buf_index += bytes_this_page;
                addr += bytes_this_page;
                bytes_remaining -= bytes_this_page;
                dbg(DBG_PRINT, "(GRADING3A)\n");
        }

        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 0;
}

/* Write from 'buf' into the virtual address space of 'map' starting at
 * 'vaddr' for size 'count'. To do this, you will need to find the correct
 * vmareas to write into, then find the correct pframes within those vmareas,
 * and finally write into the physical addresses that those pframes correspond
 * to. You should not check permissions of the areas you use. Assume (KASSERT)
 * that all the areas you are accessing exist. Remember to dirty pages!
 * Returns 0 on success, -errno on error.
 */
int
vmmap_write(vmmap_t *map, void *vaddr, const void *buf, size_t count)
{
        // KASSERT(NULL != map);
        // KASSERT(NULL != vaddr);
        // KASSERT(NULL != buf);
        // KASSERT(count > 0);

        uint32_t buf_index = (uint32_t)buf;
        uint32_t addr = (uint32_t)vaddr;
        uint32_t bytes_remaining = count;

        while (bytes_remaining > 0) {
                vmarea_t *vma = vmmap_lookup(map, ADDR_TO_PN(addr));
                // if (vma == NULL) {
                //         dbg(DBG_PRINT, "(GRADING3? vwrite1)\n");
                //         return -EFAULT;
                // }
                KASSERT(vma);

                uint32_t page_num = ADDR_TO_PN(addr) + vma->vma_off - vma->vma_start;
        
                pframe_t *pframe;
                int err = pframe_lookup(vma->vma_obj, page_num, 0, &pframe);
                // if (err < 0) {
                //         dbg(DBG_PRINT, "(GRADING3? vwrite2)\n");
                //         return err;
                // }

                uint32_t page_offset = PAGE_OFFSET(addr);
                uint32_t bytes_this_page = PAGE_SIZE - page_offset;
                if (bytes_this_page > bytes_remaining) {
                        bytes_this_page = bytes_remaining;
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                }

                // Copy FROM buffer TO page frame
                memcpy((char *)pframe->pf_addr + page_offset,
                      (const void *)buf_index, 
                      bytes_this_page);
              
                pframe_dirty(pframe);
    
                buf_index += bytes_this_page;
                addr += bytes_this_page;
                bytes_remaining -= bytes_this_page;
                dbg(DBG_PRINT, "(GRADING3A)\n");
        }

        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 0;
}
