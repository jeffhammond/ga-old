/* $Id: rmw.c,v 1.12 2001-07-31 19:31:38 d3h325 Exp $ */
#include "armcip.h"
#include "locks.h"
#include "copy.h"
#include <stdio.h>

/* enable use of newer interfaces in SHMEM */
#ifndef CRAY
/* manpages for shmem_fadd exist on the T3E but library code does not */
#define SHMEM_FADD 
#endif

/* global scope to prevent compiler optimization of volatile code */
int  _a_temp;
long _a_ltemp;

void armci_generic_rmw(int op, void *ploc, void *prem, int extra, int proc)
{
    int lock = proc%NUM_LOCKS;

    NATIVE_LOCK(lock);
    switch (op) {
      case ARMCI_FETCH_AND_ADD:
                armci_get(prem,ploc,sizeof(int),proc);
                _a_temp = *(int*)ploc + extra;
                armci_put(&_a_temp,prem,sizeof(int),proc);
           break;
      case ARMCI_FETCH_AND_ADD_LONG:
                armci_get(prem,ploc,sizeof(long),proc);
                _a_ltemp = *(long*)ploc + extra;
                armci_put(&_a_ltemp,prem,sizeof(long),proc);
           break;
      case ARMCI_SWAP:
                armci_get(prem,&_a_temp,sizeof(int),proc);
                armci_put(ploc,prem,sizeof(int),proc);
                *(int*)ploc = _a_temp; 
           break;
      case ARMCI_SWAP_LONG:
                armci_get(prem,&_a_ltemp,sizeof(long),proc);
                armci_put(ploc,prem,sizeof(long),proc);
                *(long*)ploc = _a_ltemp;
           break;
      default: armci_die("rmw: operation not supported",op);
    }

    ARMCI_Fence(proc); /* we need fence before unlocking */
    NATIVE_UNLOCK(lock);
}


int ARMCI_Rmw(int op, int *ploc, int *prem, int extra, int proc)
{
#ifdef LAPI64
    extern int LAPI_Rmw64(lapi_handle_t hndl, RMW_ops_t op, uint tgt, 
             long long *tgt_var,
             long long *in_val, long long *prev_tgt_val, lapi_cntr_t *org_cntr);
    long long llval, *pllarg = (long long*)ploc, lltmp;
#define RMWBROKEN 
#endif
#ifdef LAPI
    int  ival, rc, opcode=SWAP, *parg=ploc;
    lapi_cntr_t req_id;
#elif defined(_CRAYMPP) || defined(QUADRICS)
    int  ival;
    long lval;
#endif

#if defined(LAPI64) && defined(RMWBROKEN)
/* hack for rmw64 BROKEN: we operate on least significant part of long */
if(op==ARMCI_FETCH_AND_ADD_LONG || op==ARMCI_SWAP_LONG){
  ploc[0]=0;
  ploc[1]=0;
  ploc++;
  parg ++; prem++;
}
#endif

#if defined(CLUSTER) && !defined(LAPI) && !defined(QUADRICS)
     if(!SAMECLUSNODE(proc)){
       armci_rem_rmw(op, ploc, prem,  extra, proc);
       return 0;
     }
#endif


    switch (op) {
#   if defined(QUADRICS) || defined(_CRAYMPP)
      case ARMCI_FETCH_AND_ADD:
#ifdef SHMEM_FADD
          *(int*) ploc = shmem_int_fadd(prem, extra, proc);
#else
          while ( (ival = shmem_int_swap(prem, INT_MAX, proc) ) == INT_MAX);
          (void) shmem_int_swap(prem, ival +extra, proc);
          *(int*) ploc = ival;
#endif
        break;
      case ARMCI_FETCH_AND_ADD_LONG:
#ifdef SHMEM_FADD
          *(long*) ploc = shmem_long_fadd( (long*)prem, (long) extra, proc);
#else
          while ((lval=shmem_long_swap((long*)prem,LONG_MAX,proc)) == LONG_MAX);
          (void) shmem_long_swap((long*)prem, (lval + extra), proc);
          *(long*)ploc   = lval;
#endif
        break;
      case ARMCI_SWAP:
          *(int*)ploc = shmem_int_swap((int*)prem, *(int*)ploc,  proc); 
        break;
      case ARMCI_SWAP_LONG:
          *(long*)ploc = shmem_swap((long*)prem, *(long*)ploc,  proc); 
        break;
#   elif defined(LAPI)
#     if defined(LAPI64) && !defined(RMWBROKEN)
        case ARMCI_FETCH_AND_ADD_LONG:
           opcode = FETCH_AND_ADD;
           lltmp  = (long long)extra;
           pllarg = &lltmp;
        case ARMCI_SWAP_LONG:
#if 1
          printf("before opcode=%d rem=%ld, loc=(%ld,%ld) extra=%ld\n",
                  opcode,*prem,*(long*)ploc,llval, lltmp);  
          rc= sizeof(long);
          ARMCI_Get(prem, &llval, rc, proc);
          printf("%d:rem val before %ld\n",armci_me, llval); fflush(stdout);
#endif
          if( rc = LAPI_Setcntr(lapi_handle,&req_id,0))
                        armci_die("rmw setcntr failed",rc);
          if( rc = LAPI_Rmw64(lapi_handle, opcode, proc, (long long*)prem,
                        pllarg, &llval, &req_id)) armci_die("rmw failed",rc);
          if( rc = LAPI_Waitcntr(lapi_handle, &req_id, 1, NULL))
                        armci_die("rmw wait failed",rc);

          *(long*)ploc  = (long)llval;
#if 1
          rc= sizeof(long);
          ARMCI_Get(prem, &lltmp, rc, proc);
          printf("%d:after rmw remote val from rmw=%ld and get=%ld extra=%d\n",
                  armci_me,llval, lltmp,extra);  
#endif
        break;
#     endif
      /************** here sizeof(long)= sizeof(int) **************/
      case ARMCI_FETCH_AND_ADD:
#     if !defined(LAPI64) || defined(RMWBROKEN)
        case ARMCI_FETCH_AND_ADD_LONG:
#     endif
           opcode = FETCH_AND_ADD;
           parg = &extra;
      case ARMCI_SWAP:
#     if !defined(LAPI64) || defined(RMWBROKEN)
        case ARMCI_SWAP_LONG:
#     endif
          if( rc = LAPI_Setcntr(lapi_handle,&req_id,0))
                        armci_die("rmw setcntr failed",rc);
          if( rc = LAPI_Rmw(lapi_handle, opcode, proc, prem,
                        parg, &ival, &req_id)) armci_die("rmw failed",rc);
          if( rc = LAPI_Waitcntr(lapi_handle, &req_id, 1, NULL))
                        armci_die("rmw wait failed",rc);
          *ploc  = ival;
        break;
#   else
      case ARMCI_FETCH_AND_ADD:
      case ARMCI_FETCH_AND_ADD_LONG:
      case ARMCI_SWAP:
      case ARMCI_SWAP_LONG:
           armci_generic_rmw(op, ploc, prem,  extra, proc);
        break;
#   endif
      default: armci_die("rmw: operation not supported",op);
    }

    return 0;
}

