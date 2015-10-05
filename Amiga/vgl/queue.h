/*  queue.h
 *
 *  Written by David Kessner
 *  January, 1994
 *
 * (C) 1994, David Kessner
 */

#ifndef _QUEUE_H_
#define _QUEUE_H_

/* This file is incredibally OS9 dependant, not now */
#if defined (OS9)


/* Included for size_t and NULL */
#include <stdlib.h>

/* Included for event stuff */
#include <events.h>
#include <types.h>


/* The maximum length for a queue name, including the ending NULL */
#define VGL_Q_NAME_LEN    (32)

/* The queue structure */
/* The internal structure is shared between processes */
typedef struct
  {
    char name[VGL_Q_NAME_LEN];  /* The name of the queue */
    int  number;    /* A unique number assigned to the queue            */
    int length;     /* The length of the queue, in elements             */
    size_t size;    /* The size of each element                         */
    int error;      /* The error number from the last operation         */
    int n_users;    /* A count for the number of current users          */
    int send_index; /* The sending index (where elements are put)       */
    int recv_index; /* The receiving index (where elements are removed) */
  } VGL_Q_INTERNAL;

/* This structure is not shared */
typedef struct
  {
    VGL_Q_INTERNAL *q;
    event_id       send_rdy;
    event_id       recv_rdy;
    void *data;     /* A pointer to the actual data                     */
  } VGL_Q;


/* Function prototypes, for things that are functions, anyway */
VGL_Q *vgl_q_open (char *name, int *length, size_t *size, int flags);
int vgl_q_close (VGL_Q *q);
int vgl_q_send (VGL_Q *q, void *data, int flags);
int vgl_q_recv (VGL_Q *q, void *data, int flags);
int vgl_q_stat (VGL_Q *q);
void vgl_q_clear (VGL_Q *q);
int vgl_q_error (VGL_Q *q);


/* Flags, for use in initalizing, sending and receiving */
#define Q_PRIORITY  (1<<0)   /* Put queue data ahead of everything else */
#define Q_BLOCK     (1<<1)   /* Wait for room/data before returning */
#define Q_CREATE    (1<<2)   /* The queue MUST be created when opened */
#define Q_NO_CREATE (1<<3)   /* The queue MUST NOT be created when opened */


/* Error values from vgl_q_error() */
enum
  {
    VGL_Q_ERR_NONE,           /* No Error */
    VGL_Q_ERR_MEMORY,         /* A failed malloc, or something similar */
    VGL_Q_ERR_FULL,           /* The queue is full */
    VGL_Q_ERR_EMPTY,          /* The queue is empty */
    VGL_Q_ERR_NOT_SUPPORTED,  /* Use of a function that is not supported */
    VGL_Q_ERR_QUEUE_NOT_FOUND,/* Opening of a queue that doesn't exist */
    VGL_Q_ERR_QUEUE_EXISTS,   /* Creating a queue that exists */
    VGL_Q_ERR_MAX_QUEUES,     /* The Maximum # of queues has been exceeded */
    VGL_Q_ERR_MAX_BLOCK,      /* Too many tasks are blocked per queue */
    VGL_Q_ERR_OTHER           /* Misc. catch all error */
  };

#endif

#endif

