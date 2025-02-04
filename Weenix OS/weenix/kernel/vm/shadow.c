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

#include "globals.h"
#include "errno.h"

#include "util/string.h"
#include "util/debug.h"

#include "mm/mmobj.h"
#include "mm/pframe.h"
#include "mm/mm.h"
#include "mm/page.h"
#include "mm/slab.h"
#include "mm/tlb.h"

#include "vm/vmmap.h"
#include "vm/shadow.h"
#include "vm/shadowd.h"

#define SHADOW_SINGLETON_THRESHOLD 5

int shadow_count = 0; /* for debugging/verification purposes */
#ifdef __SHADOWD__
/*
 * number of shadow objects with a single parent, that is another shadow
 * object in the shadow objects tree(singletons)
 */
static int shadow_singleton_count = 0;
#endif

static slab_allocator_t *shadow_allocator;

static void shadow_ref(mmobj_t *o);
static void shadow_put(mmobj_t *o);
static int  shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf);
static int  shadow_fillpage(mmobj_t *o, pframe_t *pf);
static int  shadow_dirtypage(mmobj_t *o, pframe_t *pf);
static int  shadow_cleanpage(mmobj_t *o, pframe_t *pf);

static mmobj_ops_t shadow_mmobj_ops = {
        .ref = shadow_ref,
        .put = shadow_put,
        .lookuppage = shadow_lookuppage,
        .fillpage  = shadow_fillpage,
        .dirtypage = shadow_dirtypage,
        .cleanpage = shadow_cleanpage
};

/*
 * This function is called at boot time to initialize the
 * shadow page sub system. Currently it only initializes the
 * shadow_allocator object.
 */
void
shadow_init()
{
        shadow_allocator = slab_allocator_create("shadow_allocator", sizeof(mmobj_t));
        KASSERT(shadow_allocator);
        dbg(DBG_PRINT, "(GRADING3A 6.a)\n");
        //NOT_YET_IMPLEMENTED("VM: shadow_init");
}

/*
 * You'll want to use the shadow_allocator to allocate the mmobj to
 * return, then then initialize it. Take a look in mm/mmobj.h for
 * macros or functions which can be of use here. Make sure your initial
 * reference count is correct.
 */
mmobj_t *
shadow_create()
{
        mmobj_t *new_shadow = (mmobj_t *)slab_obj_alloc(shadow_allocator);

        // if(new_shadow == NULL){
        //         return NULL;
        // }

        mmobj_init(new_shadow, &shadow_mmobj_ops);
        new_shadow->mmo_refcount = 1;
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return new_shadow;
        //NOT_YET_IMPLEMENTED("VM: shadow_create");
        //return NULL;
}

/* Implementation of mmobj entry points: */

/*
 * Increment the reference count on the object.
 */
static void
shadow_ref(mmobj_t *o)
{
        KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
        dbg(DBG_PRINT, "(GRADING3A 6.b)\n");
        o->mmo_refcount++;
        //NOT_YET_IMPLEMENTED("VM: shadow_ref");
}

/*
 * Decrement the reference count on the object. If, however, the
 * reference count on the object reaches the number of resident
 * pages of the object, we can conclude that the object is no
 * longer in use and, since it is a shadow object, it will never
 * be used again. You should unpin and uncache all of the object's
 * pages and then free the object itself.
 */
static void
shadow_put(mmobj_t *o)
{
        KASSERT(o && (0 < o->mmo_refcount) && (&shadow_mmobj_ops == o->mmo_ops));
        dbg(DBG_PRINT, "(GRADING3A 6.c)\n");

        if (o->mmo_refcount-1 == o->mmo_nrespages) {
                pframe_t *curr_page;
                list_iterate_begin(&o->mmo_respages, curr_page, pframe_t, pf_olink) {
                        if (curr_page->pf_pincount > 0) {  
                                pframe_unpin(curr_page);
                                dbg(DBG_PRINT, "(GRADING3A)\n");
                        }
                        if (pframe_is_dirty(curr_page)) {
                                pframe_clean(curr_page);
                                dbg(DBG_PRINT, "(GRADING3A)\n");
                        }
                        pframe_free(curr_page);
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                } 
                list_iterate_end();
                if (o->mmo_shadowed) {
                        o->mmo_shadowed->mmo_ops->put(o->mmo_shadowed);
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                }                
        }

        o->mmo_refcount--;
        if(o->mmo_refcount == 0){
                slab_obj_free(shadow_allocator, o);
                dbg(DBG_PRINT, "(GRADING3A)\n");
        }
        dbg(DBG_PRINT, "(GRADING3A)\n");
        //NOT_YET_IMPLEMENTED("VM: shadow_put");
}

/* This function looks up the given page in this shadow object. The
 * forwrite argument is true if the page is being looked up for
 * writing, false if it is being looked up for reading. This function
 * must handle all do-not-copy-on-not-write magic (i.e. when forwrite
 * is false find the first shadow object in the chain which has the
 * given page resident). copy-on-write magic (necessary when forwrite
 * is true) is handled in shadow_fillpage, not here. It is important to
 * use iteration rather than recursion here as a recursive implementation
 * can overflow the kernel stack when looking down a long shadow chain */
static int
shadow_lookuppage(mmobj_t *o, uint32_t pagenum, int forwrite, pframe_t **pf)
{
        
        mmobj_t *current = o;
        mmobj_t *bottom_obj = mmobj_bottom_obj(o);
        pframe_t* result_pframe;
        
        if (!forwrite) {
                
                while((current->mmo_shadowed)!=NULL) {
                        KASSERT(current);  
                        
                        result_pframe = pframe_get_resident(current, pagenum);
                        if (result_pframe) {
                                // if (pframe_is_busy(result_pframe)) {
                                //         sched_sleep_on(&(result_pframe)->pf_waitq);
                                //         dbg(DBG_PRINT, "(GRADING3? shadowlookuppage1)\n");
                                // }
                                *pf = result_pframe;
                                KASSERT(NULL != (*pf));
                                KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
                                dbg(DBG_PRINT, "(GRADING3A 6.d)\n");
                                return 0;
                        }
                        current = current->mmo_shadowed;
                        KASSERT(current);  
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                }
                
                
                int lookup_result = pframe_lookup(current, pagenum, 0, pf);
                if (lookup_result < 0 || *pf == NULL) {
                        dbg(DBG_PRINT, "(GRADING3D 2)\n");
                        return -1;
                }
        } 
        else {
                
                int get_result = pframe_get(current, pagenum, pf);
                if (get_result < 0) {
                        dbg(DBG_PRINT, "(GRADING3D 2)\n");
                        return get_result;
                }
        }

        KASSERT(NULL != (*pf));
        KASSERT((pagenum == (*pf)->pf_pagenum) && (!pframe_is_busy(*pf)));
        dbg(DBG_PRINT, "(GRADING3A 6.d)\n");
        return 0;
        //NOT_YET_IMPLEMENTED("VM: shadow_lookuppage");
        //return 0;
}

/* As per the specification in mmobj.h, fill the page frame starting
 * at address pf->pf_addr with the contents of the page identified by
 * pf->pf_obj and pf->pf_pagenum. This function handles all
 * copy-on-write magic (i.e. if there is a shadow object which has
 * data for the pf->pf_pagenum-th page then we should take that data,
 * if no such shadow object exists we need to follow the chain of
 * shadow objects all the way to the bottom object and take the data
 * for the pf->pf_pagenum-th page from the last object in the chain).
 * It is important to use iteration rather than recursion here as a
 * recursive implementation can overflow the kernel stack when
 * looking down a long shadow chain */
static int
shadow_fillpage(mmobj_t *o, pframe_t *pf)
{
        KASSERT(pframe_is_busy(pf));
        KASSERT(!pframe_is_pinned(pf));
        dbg(DBG_PRINT, "(GRADING3A 6.e)\n");

        pframe_t *source_pf = NULL;
        mmobj_t *bottom_obj = mmobj_bottom_obj(o);

        mmobj_t *current = o->mmo_shadowed;
        while (current != bottom_obj) {
                source_pf = pframe_get_resident(current, pf->pf_pagenum);
                if (source_pf) {
                        pframe_pin(pf);
                        memcpy(pf->pf_addr, source_pf->pf_addr, PAGE_SIZE);
                        dbg(DBG_PRINT, "(GRADING3A)\n");
                        return 0;
                }
                current = current->mmo_shadowed;
                dbg(DBG_PRINT, "(GRADING3A)\n");
        }

        pframe_pin(pf);
        int err = pframe_lookup(current, pf->pf_pagenum, 0, &source_pf);
        if (err < 0) {
                dbg(DBG_PRINT, "(GRADING3D 2)\n");
                return err;
        }
        
        memcpy(pf->pf_addr, source_pf->pf_addr, PAGE_SIZE);
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 0;
        //NOT_YET_IMPLEMENTED("VM: shadow_fillpage");
        //return 0;
}

/* These next two functions are not difficult. */

static int
shadow_dirtypage(mmobj_t *o, pframe_t *pf)
{
        pframe_set_dirty(pf);
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 0;
        //NOT_YET_IMPLEMENTED("VM: shadow_dirtypage");
        //return -1;
}

static int
shadow_cleanpage(mmobj_t *o, pframe_t *pf)
{
        pframe_clear_dirty(pf);
        dbg(DBG_PRINT, "(GRADING3A)\n");
        return 0;
        //NOT_YET_IMPLEMENTED("VM: shadow_cleanpage");
        //return -1;
}
