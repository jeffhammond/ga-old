/*
 *    Copyright (c) 2003 by Quadrics Ltd.
 *
 *    For licensing information please see the supplied COPYING file
 *
 */

/* $Id: pgs.c,v 1.1 2004-04-13 19:57:18 manoj Exp $ */
/* $Source: /tmp/hpctools/ga/armci/src/pgs.c,v $ */

#ident	"@(#)$Id: pgs.c,v 1.1 2004-04-13 19:57:18 manoj Exp $"

#include <stdlib.h>

#include <qsnet/config.h>
#include <qsnet/types.h>

#include <elan/elan.h>
#include <elan4/library.h>
#include <elan4/lib_spinlock.h>

#include <elan4/types.h>
#include <elan4/events.h>
#include <elan4/dma.h>
#include <elan4/registers.h>
#include "pgs_thread.h"

#ifndef offsetof
#define offsetof(T,F)	((int)&(((T *)0)->F))
#endif

/* Stuff from "elan_sys.h" */

/* Hardware event defines */
#define EVENT			E4_Event
#define EVENT32			E4_Event32
#define EVENT_ALIGN		E4_EVENT_ALIGN
#define EVENT_WORD		volatile E4_uint64
#define EVENT_BLK		E4_Event_Blk

#define EVENT_WORD_READY(WORD)	(*(WORD))

#define INITEVENT		elan4_initevent
#define INITEVENT_WORD		elan4_initevent_write
#define PRIMEEVENT_WORD		elan4_primeevent_write
#define RESETEVENT_WORD(WORD)	(*(WORD) = 0)		
#define INITEVENT_BLK		elan4_initevent_blk
#define PRIMEEVENT_BLK		elan4_primeevent_blk
#define POLLEVENT_WORD		elan4_pollevent_word
#define WAITEVENT_WORD		elan4_waitevent_word

/* Input Queue defines */
#define INPUT_QUEUE		E4_InputQueue
#define INPUT_QUEUE_SIZE	sizeof(E4_InputQueue)
#define INPUT_QUEUE_ALIGN	E4_INPUTQ_ALIGN
#define INPUT_QUEUE_MIN		sizeof(E4_uint64)
#define INPUT_QUEUE_MAX		2048

/* The datatype of a DMA descriptor */
#define DMA		E4_DMA
#define DMA64		E4_DMA64
#define DMA_ALIGN	E4_DMA_ALIGN

/* Memory allocation */
#define ELAN_ALLOCATOR_SIZE (128*1024)
#define MAIN_ALLOCATOR_SIZE (1024*1024)

#define ALLOC_MAIN(ALLOC, ALIGN, SIZE) elan4_allocMain(ALLOC, ALIGN, SIZE)
#define ALLOC_ELAN(ALLOC, ALIGN, SIZE) elan4_allocElan(ALLOC, ALIGN, SIZE)
#define ALLOC_VADDR(ALLOC, ALIGN, SIZE) elan4_allocVaddr(ALLOC, ALIGN, SIZE)
#define ALLOC_ELAN_VADDR(ALLOC, ALIGN, SIZE) elan4_allocElanVaddr(ALLOC, ALIGN, SIZE)

#define FREE_MAIN(R, ADDR) elan4_freeMain(ALLOC, ADDR)
#define FREE_ELAN(R, ADDR) elan4_freeElan(ALLOC, ADDR)


/* Address translation */
#define MAIN2ELAN(CTX, ADDR) elan4_main2elan(((ELAN4_CTX *)CTX), ADDR)
#define ELAN2MAIN(CTX, ADDR) elan4_elan2main(((ELAN4_CTX *)CTX), ADDR)
#define SDRAM2ELAN(CTX, ADDR) elan4_main2elan(((ELAN4_CTX *)CTX), ADDR)
#define ELAN2SDRAM(CTX, ADDR) elan4_elan2main(((ELAN4_CTX *)CTX), ADDR)

/* Default co-processor stack alignment */
#define THREAD_STACK_ALIGN	E4_STACK_ALIGN

/* Co-processor code loading */
#define LOADSO		ELAN4_LOAD
#define OPEN_SO		elan4_open_so
#define LOAD_SO		elan4_load_so
#define FIND_SYM_SO	elan4_find_sym
#define CLOSE_SO	elan4_close_so
#define UNLOAD_SO	elan4_unload_so


/* DEBUG macros */
#  define PRINTF0(STATE,m,fmt)
#  define PRINTF1(STATE,m,fmt,a)
#  define PRINTF2(STATE,m,fmt,a,b)
#  define PRINTF3(STATE,m,fmt,a,b,c)
#  define PRINTF4(STATE,m,fmt,a,b,c,d)
#  define PRINTF5(STATE,m,fmt,a,b,c,d,e)
#  define PRINTF6(STATE,m,fmt,a,b,c,d,e,f)
#  define PRINTF7(STATE,m,fmt,a,b,c,d,e,f,g)
#  define PRINTF8(STATE,m,fmt,a,b,c,d,e,f,g,h)


/* Ripped off event_sys.h */

/* 
 * Generic libelan EVENT managament functions
 *
 */

typedef enum eventState
{
    EVENT_BAD = 0,    /* Unconfigured */
    EVENT_LIVE_LIVE,  /* In the hands of a higher level library - possibly ourselfs */
    EVENT_LIVE_DONE,  /* In the hands of a higher level library but known finished */
    EVENT_FREE        /* Idle */
} EVENTSTATE;

/* Generic event structure returned by libelan interfaces.
 *
 * Currently there is 1.5 pointers worth of padding in this struct.
 *
 * Many [poll|wait]Fns simply call elan_pollWord (look in queueTx.c)
 * so it might be an idea to have the argumnets to elan_pollWord
 * embedded in this struct instead of poll/waitFn.  This would
 * need 2.5 pointers (RAIL, event, doneWord) however they could exist
 * in a union with the poll/waitFn.
 * 
 * The "handle" could be recoverd to store a pointer in too if needed.
 *
 * I would need to borrow a bit from the "state" to store a type bit to
 * implement this.
 */
typedef struct event_main
{
    void               *handle;                  /* Generic handle */
    int (*pollFn)(ELAN_EVENT *ready, long how);  /* fn to poll for completion */
    void (*waitFn)(ELAN_EVENT *ready, long how); /* fn to wait for completion */
    void (*freeFn)(ELAN_EVENT *ready);           /* fn to free off descriptor when done */
    struct event_main  *evm_next;
    enum eventState     e_state;                /* State (word) */

}		EVENT_MAIN;

typedef int (*EVENT_POLLFN)(ELAN_EVENT *ready, long how);
typedef void (*EVENT_WAITFN)(ELAN_EVENT *ready, long how);
typedef void (*EVENT_FREEFN)(ELAN_EVENT *ready);

extern int	elan_pollWord (ELAN_STATE *elan_state, ELAN_RAIL *rail,
			       EVENT_WORD *ready, long how);
extern void	elan_waitWord (ELAN_STATE *elan_state, ELAN_RAIL *rail,
			       void *readyEvent, EVENT_WORD *ready, long how);

/* Called every time it's before it's released to the user */
#define ELAN_EVENT_PRIME(X)   (((EVENT_MAIN *)X)->e_state = EVENT_LIVE_LIVE);

/* End of event_sys.h */

//#include "putget_sys.h"
#include "pgs_sys.h"

static void _elan_pgsFree (ELAN_EVENT *event);
static void _elan_pgsWait (ELAN_EVENT *event, long how);
static int  _elan_pgsPoll (ELAN_EVENT *event, long how);

int elan_pgsGlobalMemSize (ELAN_STATE *state)
{
    return(INPUT_QUEUE_SIZE);
}

void pgs_getStats(pgsstate_t *state, ELAN_PUTGETSTATS *stats)
{
}

#define idivide(x,y) ((y) ? (x)/(y) : 0)





/*
 * allocate and initialise our request
 */
int pgs_initDesc (ELAN_STATE *state, ELAN_RAIL *rail, void *handle,
		  void *main, ADDR_SDRAM elan)
{
    PGS_REQDESC *r = (PGS_REQDESC *)main;
    PGS_REQELAN *re;
    PGS_RAIL *pgsrail = (PGS_RAIL*)handle;
    PGS_REQ *req;
    DMA64 *qdma;

    r->r_elan = re = elan;
    r->r_dests = 0;
    r->r_rail = pgsrail;

    INITEVENT_WORD(rail->rail_ctx, (E4_Event *)&re->re_doneEvent, &r->r_done);
    PRIMEEVENT_WORD(rail->rail_ctx, (E4_Event *)&re->re_doneEvent, 1);
    
    req = &r->r_req;
    req->req_doneEvent = MAIN2ELAN(rail->rail_ctx, &re->re_doneEvent);
    req->req_srcvp = state->vp;
    
    /*
     * allocate and initialise the QDMA 
     */
    qdma = &r->r_qdma;
    qdma->dma_srcAddr  = MAIN2ELAN(rail->rail_ctx, req);
    qdma->dma_dstAddr  = 0; /* Queue slot offset */
    qdma->dma_srcEvent = 0;
    qdma->dma_dstEvent = pgsrail->pr_state->pgs_qaddr;
    
    r->r_event.handle = pgsrail;
    r->r_event.pollFn = _elan_pgsPoll;
    r->r_event.waitFn = _elan_pgsWait;
    r->r_event.freeFn = _elan_pgsFree;
 
    return(TRUE);
}


void pgs_railInit (pgsstate_t *pgsstate, PGS_RAIL *pgsrail, int nSlots)
{
    ELAN_RAIL *rail = pgsrail->pr_rail;
    ELAN_ESTATE *estate;
    E4_ThreadRegs tregs;
    E4_CmdQ   *tcmdq;
    PGS_ELAN  *pe;
    
    pgsrail->pr_state = pgsstate;

    /* Create an SDRAM allocator */
    if ((pgsrail->pr_sdram = elan4_open_sdram (pgsrail->pr_id, 0, ELAN_ALLOCATOR_SIZE)) == NULL)
    {
	printf ("Failed: elan4_open_sdram\n");
	exit (-1);
    }
    
    /* Create a Main/Elan memory allocator */
    if ((pgsrail->pr_alloc = elan4_createAllocator (MAIN_ALLOCATOR_SIZE, pgsrail->pr_sdram, 0, ELAN_ALLOCATOR_SIZE)) == NULL) {
	printf ("Failed: elan4_createAllocator\n");
	exit (-1);
    }

#if 0
    /*
     * Setup for descriptor allocation 
     */
    _elan_caRegister(pgsstate->elan_state,
		     &pgsrail->pr_ca,		          /* control      */
		     rail,	                          /* rail         */
		     "PGS_REQDESC",	                  /* name         */
		     offsetof(PGS_REQDESC, r_event),
		     sizeof(PGS_REQDESC), ELAN_ALIGN,     /* main         */
		     sizeof(PGS_REQELAN), ELAN_ALIGN,     /* elan         */
		     pgs_initDesc,	                  /* initFn       */
		     CA_AUTODESTROY,		          /* destroyFn    */
		     pgsrail,			          /* handle       */
		     4, 4, 128,	   	                  /* init, add (0 = double), max */
		     DBG_PGS);                            /* debug flags  */
#endif

    /* Allocate a cookie pool */
    pgsrail->pr_cpool = elan4_allocCookiePool(rail->rail_ctx, pgsstate->elan_state->vp);

    /* 
     * allocate Elan control structure
     */
    if (!(pe = ALLOC_ELAN(pgsrail->pr_alloc, ELAN_ALIGN, ELAN_ALIGNUP(sizeof(PGS_ELAN), ELAN_ALIGN))))
	elan_exception(pgsstate->elan_state, ELAN_ENOMEM, "pgs_init: Elan memory exhausted");
    pgsrail->pr_elan = pe;

    memset(pe, 0, sizeof(PGS_ELAN));
    
    /*
     * Allocate a command queue for issuing DMAs/STEN packets
     * and running pgs_thread
     */
    if ((pgsrail->pr_cmdq = elan4_alloc_cmdq(rail->rail_ctx, 
                                             pgsrail->pr_alloc, CQ_Size8K,
					     CQ_ThreadStartEnableBit |
					     CQ_WriteEnableBit |
					     CQ_DmaStartEnableBit | 
                                             CQ_STENEnableBit,
					     NULL)) == NULL)
	elan_exception (pgsstate->elan_state, ELAN_EINTERNAL,
			"pgs_init: failed to allocate a command queue");
    
    /*
     * Allocate a command port for the thread
     */
#define PGS_CMDQ_WORK_PER_QSLOT 10
    {
	unsigned cqsize = elan4_cqsize_inputq_ctrlflow (0, PGS_CMDQ_WORK_PER_QSLOT, nSlots);
	if ((tcmdq = elan4_alloc_cmdq(rail->rail_ctx, pgsrail->pr_alloc, cqsize, CQ_EnableAllBits, NULL)) == NULL)
	    elan_exception (pgsstate->elan_state, ELAN_EINTERNAL,
			    "pgs_init: failed to allocate thread command queue on rail(rail %p)\n", rail);

	/* Translate CQ address for thread use */
	pe->pe_work_cmdq = MAIN2ELAN (rail->rail_ctx, tcmdq->cmdq_mapping);
	/* allocate a shared cmdq for thread rescheduling */
	if ((pe->pe_sched_cmdq = elan4_alloccq_space(rail->rail_ctx, 8, CQ_Size1K)) == 0)
	    elan_exception (pgsstate->elan_state, ELAN_ENOMEM,
			    "pgs_init: failed to alloc shared command queue resource rail ctx %p",
			    rail->rail_ctx);
    }

    /*
     * Copy down Elan pgs state
     */
    estate = (ELAN_ESTATE *)pgsstate->elan_state->estate;
    pe->pe_state = MAIN2ELAN(rail->rail_ctx, estate);
    pe->pe_shared = MAIN2ELAN(rail->rail_ctx, &pgsrail->pr_shared);

    /* initialise and prime the thread started event */
    INITEVENT_WORD(rail->rail_ctx, (E4_Event *)&pe->pe_started, &pgsrail->pr_started);
    PRIMEEVENT_WORD(rail->rail_ctx, (E4_Event *)&pe->pe_started, 1);

    /* Zero the InputQueue */
    pgsrail->pr_q = ELAN2MAIN(rail->rail_ctx, pgsstate->pgs_qaddr);
    memset(pgsrail->pr_q, 0, sizeof(INPUT_QUEUE)); 
    
    /*
     * Now initialise the Elan Input Queue structure
     * We create this in Elan memory as it will be the Elan thread
     * that has to read the envelope information
     */
    if (!(pgsrail->pr_qbase = ALLOC_ELAN(pgsrail->pr_alloc, ELAN_ALIGN, nSlots * PGS_QSLOTSIZE)))
	elan_exception (pgsstate->elan_state, ELAN_ENOMEM, "pgs_init: Elan memory exhausted");
    
    pgsrail->pr_qtop = (char *)pgsrail->pr_qbase + ((nSlots-1) * PGS_QSLOTSIZE);
    
    /* Write Queue address to the Elan */
    pe->pe_qaddr = pgsstate->pgs_qaddr;
    
    /* Stash the queue FirstItem/LastItem for receiver thread use */
    pe->pe_qbase = MAIN2ELAN(rail->rail_ctx, pgsrail->pr_qbase);
    pe->pe_qtop = MAIN2ELAN(rail->rail_ctx, pgsrail->pr_qtop);
    
    pgsrail->pr_q->q_bptr = MAIN2ELAN(rail->rail_ctx, pgsrail->pr_qbase);
    pgsrail->pr_q->q_fptr = MAIN2ELAN(rail->rail_ctx, pgsrail->pr_qbase);
    
    pgsrail->pr_q->q_control = 
	    E4_InputQueueControl (MAIN2ELAN(rail->rail_ctx, pgsrail->pr_qbase),
				  MAIN2ELAN(rail->rail_ctx, pgsrail->pr_qtop), PGS_QSLOTSIZE);
	     
#ifdef DEBUG
    /* Fill Input queue slots with silly pattern */
    memset(pgsrail->pr_qbase, 0xeb, nSlots * PGS_QSLOTSIZE);
#else
    memset(pgsrail->pr_qbase, 0, nSlots * PGS_QSLOTSIZE);
#endif
    
    /* Allocate queue event */
    if (!(pgsrail->pr_qevent = (EVENT *) ALLOC_ELAN(pgsrail->pr_alloc, 64, sizeof(E4_Event32))))
	elan_exception (pgsstate->elan_state, ELAN_ENOMEM, "pgs_init: Elan memory exhausted:");
    
    /* Initialise the event */
    pgsrail->pr_qevent->ev_CountAndType = 0;
    pgsrail->pr_qevent->ev_CopySource = 0;
    pgsrail->pr_qevent->ev_CopyDest =  0;
    
    pgsrail->pr_q->q_event = MAIN2ELAN(rail->rail_ctx, pgsrail->pr_qevent);


#if 0
    elan_dbg(pgsstate->elan_state, DBG_INIT | DBG_PGS,
	     "pgs_init: queue %lx q_bptr %llx q_control %llx q_event %lx(%llx)\n",
	     pgsrail->pr_q, pgsrail->pr_q->q_bptr,pgsrail->pr_q->q_control,
	     pgsrail->pr_qevent, pgsrail->pr_q->q_event);
#endif
    
    /* 
     * Allocate thread stack 
     */
    if (!(pgsrail->pr_stack = ALLOC_ELAN(pgsrail->pr_alloc, THREAD_STACK_ALIGN, PGS_THREADSTACK)))
	elan_exception (pgsstate->elan_state, ELAN_ENOMEM, 
			"pgsInit: Elan memory exhausted: stack allocate: pgs %p", pgsstate);
    

    /*
     * Load the co-processor thread 
     */
    {
	void *cspace;
	LOADSO *code;
	char file_name[20];
	FILE *fp;
	int sz;
	extern int armci_msg_me();

	/* hack: creating binary file in working directory */
	sprintf(file_name, "./PGS1234%d.so", armci_msg_me()); 
	fp = fopen((const char*)file_name, "w");
	if(fp==NULL){printf("cannot open pgs_thread.so\n");exit(-1);}
	sz = fwrite(bin_data, 1, BIN_DATA_ELEMS, fp);
	if(sz<BIN_DATA_ELEMS) {printf("pgs:file write error\n");exit(-1);}
	fclose(fp);
	
	if ((code = pgsrail->pr_pgsCode = OPEN_SO(rail->rail_ctx,
						  (const char*)file_name,
						  //"libelan_thread.so",
						  NULL, 0)) == NULL)
	    exit (-1);
	
	cspace = elan4_allocElan (pgsrail->pr_alloc, 8192, code->loadsize);

	if (LOAD_SO(code, cspace, elan4_main2elan (rail->rail_ctx, cspace)) < 0)
	    exit (-1);
	
	pgsrail->pr_pgsSym  = elan4_find_sym (code, "pgs_thread");
	unlink((const char*)file_name);
    }

#if 0
    elan_dbg(pgsstate->elan_state, DBG_INIT | DBG_PGS,
	     "pgs_init: loaded lib @ %p(%Lx) : 'pgs_thread' symbol @ %Lx\n",
	     pgsrail->pr_pgsCode->loadbase, pgsrail->pr_pgsCode->loadaddr, rail->r_pgsSym);
    
    elan_dbg(pgsstate->elan_state, DBG_INIT | DBG_PGS,
	     "pgs_init: starting elan pgs thread (%Lx) - stack %p args %Lx\n",
	     rail->r_pgsSym, pgsrail->pr_stack, MAIN2ELAN(rail->rail_ctx, pe));
#endif
    
    /* Create the initial thread register state */
    elan4_init_thread(rail->rail_ctx, &tregs, pgsrail->pr_pgsSym,
		      pgsrail->pr_stack, PGS_THREADSTACK,
		      /* nargs */ 1, 
		      MAIN2ELAN(rail->rail_ctx, pe));
        
    /* 
     * Start the thread and wait until it sets the started event
     */
    elan4_run_thread_cmd(pgsrail->pr_cmdq, &tregs);
    pgsrail->pr_cmdq->cmdq_flush (pgsrail->pr_cmdq);

    /* Wait for thread to get started */
    elan_waitWord(pgsstate->elan_state, rail, &pe->pe_started, &pgsrail->pr_started, ELAN_POLL_EVENT);

#if 0
    elan_dbg(pgsstate->elan_state, DBG_INIT | DBG_PGS, "pgs_init: thread started\n");
#endif
}

void *
pgs_init (ELAN_STATE *state, void *qMem)
{
    pgsstate_t *pgsstate;
    int r;
    
    /*
     * setup the pgs state for this putget control
     */
    if ((pgsstate = (pgsstate_t *)calloc(1, sizeof(pgsstate_t))) == NULL)
	elan_exception(state, ELAN_ENOMEM, "pgs_init: failed to allocate memory");
    
    pgsstate->elan_state = state;
    pgsstate->pgs_vp = state->vp;
    
    pgsstate->pgs_waitType = (elan_base ? elan_base->waitType : ELAN_POLL_EVENT);
    
    /* Stash the main and Elan Queue addresses */
    pgsstate->pgs_qaddr = (ADDR_ELAN) ((uintptr_t) qMem);
    
    /*
     * initialisation each rail
     */
    pgsstate->pgs_nrails = elan_nRails(pgsstate->elan_state);
    if ((pgsstate->pgs_rails = (PGS_RAIL *)calloc(pgsstate->pgs_nrails, sizeof(PGS_RAIL))) == NULL)
	elan_exception(pgsstate->elan_state, ELAN_ENOMEM, "pgs_init: failed to allocate memory");
    for (r=0; r < pgsstate->pgs_nrails; r++)
    {
	PGS_RAIL *pr = pgsstate->pgs_rails + r;
	pr->pr_id = r;
	pr->pr_rail = pgsstate->elan_state->rail[r];
	
	pgs_railInit(pgsstate, pr, 16);
    }

    pgsstate->pgs_profile = 0;

#if 0
    printf("pgs initialized\n"); fflush(stdout);
#endif
    return pgsstate;
}

static ELAN_EVENT *issueStridedRequest(PGS_RAIL *pgsr, PGS_REQDESC *r)
{
    PGS_REQ *req;
    DMA64 *qdma;

    MUTEX_LOCK(&pgsr->pr_mutex);
    
    qdma = &r->r_qdma;
    req = &r->r_req;
    
    /* 
     * Update the request 
     */
    qdma->dma_typeSize = E4_DMA_TYPE_SIZE(r->r_qdmasize, DMA_DataTypeByte, DMA_QueueWrite, 16);
    
    /* 
     * Update the DMA descriptor 
     */
    qdma->dma_cookie = elan4_local_cookie(pgsr->pr_cpool, E4_COOKIE_TYPE_LOCAL_DMA, r->r_vp);
    qdma->dma_vproc = r->r_vp;
    
    /* 
     * Send the QDMA 
     */
    elan4_run_dma_cmd(pgsr->pr_cmdq, (DMA *)qdma);
    pgsr->pr_cmdq->cmdq_flush(pgsr->pr_cmdq);

    MUTEX_UNLOCK(&pgsr->pr_mutex);
    
    ELAN_EVENT_PRIME(&r->r_event);

    return(&r->r_event);
}




static void pack_sdscr(void *src, void *dst, int *src_stride_arr, int *dst_stride_arr, u_int *count, u_int strides, u_int destvp, PGS_REQ *r, ELAN_RAIL *rail)
{
int i;

    r->src = MAIN2ELAN(rail->rail_ctx, src);
    r->dst = MAIN2ELAN(rail->rail_ctx, dst);
    r->strides = (E4_uint32)strides;
    for(i=0; i<strides; i++)r->src_stride_arr[i] = (E4_uint32)src_stride_arr[i];
    for(i=0; i<strides; i++)r->dst_stride_arr[i] = (E4_uint32)dst_stride_arr[i];
    for(i=0; i<strides+1; i++)r->count[i] = (E4_uint32)count[i];
}


PGS_REQDESC *
ACQUIRE_LH (PGS_RAIL *pgsrail)
{
    PGS_REQDESC *r, *rbase;
    PGS_REQELAN *re;
    
    int i, growth = 16;

    if ((r = pgsrail->pr_freeDescs))
    {
	pgsrail->pr_freeDescs = r->r_suc;
	r->r_suc = NULL;
	return r;
    }
    
    /* New to grow pool of free descs */
    r = rbase = ALLOC_MAIN(pgsrail->pr_alloc, ELAN_ALIGN, sizeof(PGS_REQDESC) * growth);
    re = ALLOC_ELAN(pgsrail->pr_alloc, ELAN_ALIGN, sizeof(PGS_REQELAN) * growth);

    for (i = 0; i < growth; i++, r++, re++)
    {
	/* Initialise each newly allocated desc */
	pgs_initDesc(pgsrail->pr_state->elan_state, pgsrail->pr_rail, pgsrail, r, re);
	
	/* Save all but first on freeList */
	if (i)
	{
	    r->r_suc = pgsrail->pr_freeDescs;
	    pgsrail->pr_freeDescs = r;
	}
    }
    
    return rbase;
}

void
RELEASE_LH (PGS_REQDESC *r)
{
    PGS_RAIL *pgsrail = r->r_rail;

    r->r_suc = pgsrail->pr_freeDescs;
    pgsrail->pr_freeDescs = r;
}

ELAN_EVENT *elan_putss (void *pgs, void *src, void *dst, int *src_stride_arr, 
                 int *dst_stride_arr, u_int *count, u_int strides, u_int destvp)
{
    int rail=0 ;
    pgsstate_t *pgsstate = (pgsstate_t *) pgs;
    ELAN_EVENT *event = 0;
    PGS_REQDESC *rdesc;

    if(strides<0 || strides> MAXSTRIDE){
       fprintf(stderr,"error in strided put: bad stride %d\n",strides);
       return(event);
    }

#if 0
    pgsstate->pgs_putscalls++;
    bytes = count[0];
    for(i=0; 1<strides; i++)
      bytes *= count[i];
    pgsstate->pgs_putsbytes += bytes;

    if (bytes == 0)
    {
	event = elan_nullEvent(pgsstate->elan_state);
	return(event);
    }
#endif

    for(rail = 0; rail <1; rail++){ /* for now only 1 rail */
       PGS_RAIL *pgsr = pgsstate->pgs_rails + rail;
       if (!(rdesc = (PGS_REQDESC *)ACQUIRE_LH(pgsr)))
   	   elan_exception(pgsr->pr_state->elan_state, ELAN_ENOMEM,
		       "elan_putss: failed to allocate descriptor");

       pack_sdscr(src, dst, src_stride_arr, dst_stride_arr, count, strides, 
                  destvp, &rdesc->r_req, pgsr->pr_rail);
   
       
       rdesc->r_qdmasize = STRIDEDHSIZE;
       rdesc->r_vp = pgsstate->elan_state->vp; /* where to send the PUTS desc */
       rdesc->r_req.req_type = PGS_PUTS;
       rdesc->r_req.req_rvp = destvp; /* where to send the data to */
       event = elan_link(event, issueStridedRequest(pgsr, rdesc)); 
    }

    return(event);
}

/*\ extra can be scale factor, esize is sizeof(extra)
\*/
ELAN_EVENT *elan_accs (void *pgs, void *src, void *dst, int *src_stride_arr, 
                 int *dst_stride_arr, u_int *count, u_int strides, 
                 void *extra, int esize, u_int destvp)
{
    int rail=0 ;
    pgsstate_t *pgsstate = (pgsstate_t *) pgs;
    ELAN_EVENT *event = 0;
    PGS_REQDESC *rdesc;

    if(strides<0 || strides> MAXSTRIDE){
       fprintf(stderr,"error in strided put: bad stride %d\n",strides);
       return(event);
    }

#if 0
    pgsstate->pgs_putscalls++;
    bytes = count[0];
    for(i=0; 1<strides; i++) bytes *= count[i];
    pgsstate->pgs_putsbytes += bytes;

    if (bytes == 0) {
        event = elan_nullEvent(pgsstate->elan_state);
        return(event);
    }
#endif

    for(rail = 0; rail <1; rail++){ /* for now only 1 rail */
       PGS_RAIL *pgsr = pgsstate->pgs_rails + rail;
       if (!(rdesc = (PGS_REQDESC *)ACQUIRE_LH(pgsr)))
           elan_exception(pgsr->pr_state->elan_state, ELAN_ENOMEM,
                       "elan_putss: failed to allocate descriptor");

       pack_sdscr(src, dst, src_stride_arr, dst_stride_arr, count, strides,
                  destvp, &rdesc->r_req, pgsr->pr_rail);
  
       rdesc->r_qdmasize = STRIDEDHSIZE;
       rdesc->r_vp = destvp; /* where to send the ACC desc (rem node) */
       rdesc->r_req.req_type = PGS_ACC;
       rdesc->r_req.req_rvp = pgsstate->elan_state->vp; /* source data */
       event = elan_link(event, issueStridedRequest(pgsr, rdesc));
    }

    return(event);
}



ELAN_EVENT *elan_getss (void *pgs, void *src, void *dst, int *src_stride_arr, int *dst_stride_arr, u_int *count, u_int strides, u_int destvp)
{
    int rail=0 ;
    pgsstate_t *pgsstate = (pgsstate_t *) pgs;
    ELAN_EVENT *event = 0;
    PGS_REQDESC *rdesc;

    if(strides<0 || strides> MAXSTRIDE){
       fprintf(stderr,"error in strided put: bad stride %d\n",strides);
       return(event);
    }

#if 0
    pgsstate->pgs_putscalls++;
    bytes = count[0];
    for(i=0; 1<strides; i++) bytes *= count[i];
    pgsstate->pgs_putsbytes += bytes;

    if (bytes == 0) {
	event = elan_nullEvent(pgsstate->elan_state);
	return(event);
    }
#endif

    for(rail = 0; rail <1; rail++){ /* for now only 1 rail */
       PGS_RAIL *pgsr = pgsstate->pgs_rails + rail;
       if (!(rdesc = (PGS_REQDESC *)ACQUIRE_LH(pgsr)))
   	   elan_exception(pgsr->pr_state->elan_state, ELAN_ENOMEM,
		       "elan_putss: failed to allocate descriptor");

       pack_sdscr(src, dst, src_stride_arr, dst_stride_arr, count, strides, 
                  destvp, &rdesc->r_req, pgsr->pr_rail);
       
       rdesc->r_qdmasize = STRIDEDHSIZE;
       rdesc->r_vp = destvp; /* where to send the PUTS desc to */
       rdesc->r_req.req_type = PGS_PUTS;
       rdesc->r_req.req_rvp = pgsstate->elan_state->vp; /* where to send the data to */
       event = elan_link(event, issueStridedRequest(pgsr, rdesc)); 
    }

    return(event);
}



static int _elan_pgsPoll (ELAN_EVENT *event, long how)
{
    PGS_REQDESC *r = (PGS_REQDESC*)event;
    PGS_RAIL    *pgsrail = r->r_event.handle;
    ELAN_RAIL   *rail = pgsrail->pr_rail;
    ELAN_STATE  *state = pgsrail->pr_state->elan_state;
    
    PRINTF2(state, DBG_PGS, "_elan_pgsPoll(%p) polling how=%x magic=%x\n", event, how);
    
    if (!elan_pollWord(state, rail, &r->r_done, how))
    {
	PRINTF1(state, DBG_PGS, "_elan_pgsPoll(%p) returning FALSE\n", event);
	return(FALSE);
    }
    return(TRUE);
}


static void _elan_pgsWait (ELAN_EVENT *event, long how)
{
    PGS_REQDESC *r = (PGS_REQDESC*)event;
    PGS_RAIL    *pgsrail = r->r_event.handle;
    ELAN_RAIL   *rail = pgsrail->pr_rail;
    ELAN_STATE  *state = pgsrail->pr_state->elan_state;
    
    PRINTF2(state, DBG_PGS, "_elan_pgsWait(%p) waiting on event %p\n", event, &r->r_elan->re_doneEvent);
    
    elan_waitWord(state, rail, &r->r_elan->re_doneEvent, &r->r_done, how);
    
    return;
}

static void _elan_pgsFree (ELAN_EVENT *event)
{
    PGS_REQDESC *r = (PGS_REQDESC*)event;
    PGS_RAIL    *pgsrail = r->r_event.handle;
    ELAN_RAIL   *rail = pgsrail->pr_rail;

    // clear the descriptor for reuse
    r->r_dests = 0;

    MUTEX_LOCK(&pgsrail->pr_mutex);
    
    // reprime the software event
    r->r_done = 0;
    PRIMEEVENT_WORD(rail->rail_ctx, (E4_Event *)&r->r_elan->re_doneEvent, 1);
    
    RELEASE_LH(r);
    
    MUTEX_UNLOCK(&pgsrail->pr_mutex);
}


/*
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
