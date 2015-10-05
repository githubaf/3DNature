/*  queue.c -- message queue handling functions.
 *
 *  Written by David Kessner
 *  January, 1994
 *
 * (C) 1994, David Kessner
 */


#if defined (OS9)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <module.h>
#include <modes.h>
#include <procid.h>
#include <setsys.h>
#include <errno.h>

#include "../os9/common/sys_os9.h"
#endif


#include "queue.h"


/* These files are included for their clib type functions */
#include "vgl.h"
#include "vgl_internals.h"
#include "vgl_config.h"


#ifdef USE_VGL_QUEUE

/* Certain "standard" functions, and their definitions.  */

#define _q_malloc(size)              vgl_malloc(size)
#define _q_free(ptr)                 vgl_free(ptr)
#define _q_strcmp(aaa,bbb)           vgl_stricmp(aaa,bbb)
#define _q_strcpy(dest,source)       vgl_strcpy(dest,source)
#define _q_strcat(dest,source)       vgl_strcat(dest,source)
#define _q_memcpy(dest,source,size)  vgl_memcpy(dest,source,size)

#define _q_min(aaa,bbb)   (((aaa)<(bbb))?(aaa):(bbb))
#define _q_max(aaa,bbb)   (((aaa)>(bbb))?(aaa):(bbb))

#define _q_mask_irq()
#define _q_unmask_irq()


/* These "functions" are defined as macros for speed */
#define _q_stat_inline(qqq)  (((qqq)->send_index >= (qqq)->recv_index) ? \
			      ((qqq)->send_index - (qqq)->recv_index) : \
			      ((qqq)->send_index + \
			       (qqq)->length     - \
			       (qqq)->recv_index + \
			       1))
#define _q_clear_inline(qqq) ((qqq)->recv_index = (qqq)->send_index = 0)


/* The maximum number of queues that may be "open" at any one time */
#define VGL_Q_TABLE_SIZE  (64)


/* The postfix placed on a queue data module (in OS/9) */
#define POSTFIX       "_"
#define POSTFIX_LEN   (1)


/* The error value for things without an associated queue. */
static int vgl_q_global_error = VGL_Q_ERR_NONE;


/*  I have no idea what these values actually mean.  They are used 
 *  when creating the shared data modules.  They are straight from the 
 *  Microware application notes.
 */
#define ATTR_REV   (0x8001)
#define PERMS      (0x777)


/*  Define if blocking should be done, through the use of events
 */
#define BLOCK_RECV
#undef  BLOCK_SEND



/***************************************************************************/
/***************************************************************************/
/*  Open a queue.
 *
 *  name is a unique name up to QUEUE_NAME_LEN in length.
 *  length is the number of elements or messages in the queue.
 *  size is the size of each element/message.
 *  flags is one or none of:  Q_CREATE or Q_NO_CREATE
 *
 *  Returns a pointer to the queue, or NULL on failure.  The error number
 *  can be read with vgl_q_error(NULL).
 *
 *  If a pre-existing queue is opened, the size and length of the queue is
 *  placed in *size and *length.  The size and length of the pre-existing
 *  queue is not changed once opened.
 */ 
VGL_Q *
vgl_q_open (char *name, int *length, size_t *size, int flags)
{
  int i;
  VGL_Q *q;
  mod_exec *module;
  char s[VGL_Q_NAME_LEN+POSTFIX_LEN];

  _q_mask_irq();
  

  q = _q_malloc (sizeof(VGL_Q));
  if (q==NULL)
    {
      vgl_q_global_error = VGL_Q_ERR_MEMORY;
      _q_unmask_irq();
      return(NULL);
    }

  /* Try opening a pre-existing queue */
  module = modlink (name, 0);
  if ((int)module != -1)
    {
      /* If the Q_CRETE flag is enabled then error out */
      if ((flags&Q_CREATE)!=0)
	{
	  vgl_q_global_error = VGL_Q_ERR_QUEUE_EXISTS;
          _q_unmask_irq();
	  return (NULL);
	}
      
      /* Get a pointer to Q from the module header */
      q->q = (VGL_Q_INTERNAL *)((int)module + (int)module->_mexec);

      /* Open the data module */
      _q_strcpy (s, name);
      _q_strcat (s, POSTFIX);
      module = modlink (s, 0);
      if ((int)module == -1)
	{
	  vgl_q_global_error = VGL_Q_ERR_MEMORY;
	  _q_unmask_irq();
	  return (NULL);
	}
      
      q->data = (void *)((int)module + (int)module->_mexec);

      /* Get the events */
#ifdef BLOCK_RECV
      _q_strcpy (s, name);
      _q_strcat (s, "r");
      _os_ev_link (s, &(q->recv_rdy));
#endif
#ifdef BLOCK_SEND
      _q_strcpy (s, name);
      _q_strcat (s, "s");
      _os_ev_link (s, &(q->send_rdy));
#endif

      /* The queue that was found is OK, now set some values and return... */
      *length = q->q->length;
      *size = q->q->size;
      q->q->n_users++;
      
      _q_unmask_irq();

      return (q);
    }
  else
    {
      /* If the Q_NO_CREATE flag is enabled, error out */
      if ((flags&Q_NO_CREATE)!=0)
	{
	  vgl_q_global_error = VGL_Q_ERR_QUEUE_NOT_FOUND;
	  _q_unmask_irq();
	  return (NULL);
	}
      
      /* Create a queue from "stratch" */
      module = (mod_exec *)_mkdata_module (name,
					   sizeof(VGL_Q_INTERNAL),
					   ATTR_REV,
					   PERMS);
      if ((int)module == -1)
	{
	  vgl_q_global_error = VGL_Q_ERR_MEMORY;
	  _q_unmask_irq();
	  return (NULL);
	}
      
      /* Get a pointer to Q from the module header */
      q->q = (VGL_Q_INTERNAL *)((int)module + (int)module->_mexec);

      /* Create a data module */
      _q_strcpy (s, name);
      _q_strcat (s, POSTFIX);
      module = (mod_exec *)_mkdata_module (s,
					   *length * *size,
					   ATTR_REV,
					   PERMS);
      if ((int)module == -1)
	{
	  vgl_q_global_error = VGL_Q_ERR_MEMORY;
	  _q_unmask_irq();
	  return (NULL);
	}
      
      q->data = (void *)((int)module + (int)module->_mexec);

      /* Create the events */
#ifdef BLOCK_RECV
      _q_strcpy (s, name);
      _q_strcat (s, "r");
      _os_ev_creat (-1, 1, 0, &(q->recv_rdy), s, 0, 0);
#endif
#ifdef BLOCK_SEND
      _q_strcpy (s, name);
      _q_strcat (s, "s");
      _os_ev_creat (-1, 1, 0, &(q->send_rdy), s, *length - 1, 0);
#endif

      /* Initalize the queue structure */
      _q_strcpy (q->q->name, name);
      q->q->number = -1;
      q->q->length = *length;
      q->q->size = *size;
      q->q->error = VGL_Q_ERR_NONE;
      q->q->n_users = 1;
      _q_clear_inline (q->q);             
      
      _q_unmask_irq();

      return (q);
    }
}


/***************************************************************************/
/***************************************************************************/
/*  Close an opened queue
 *
 *  Returns 0 on sucess, non-zero on failure.
 */
int
vgl_q_close (VGL_Q *q)
{
  char s[VGL_Q_NAME_LEN+POSTFIX_LEN];

  _q_mask_irq();
  
  /* Decriment the number of users for this queue */
  q->q->n_users--;
  
  /* If the number of users is zero, remove it from the table and free it */
  if (q->q->n_users <= 0)
    {

      _q_strcpy (s, q->q->name);
      _q_strcat (s, POSTFIX);

      munload (q->q->name);
      munload (s);

#ifdef BLOCK_RECV
      _os_ev_unlink (q->recv_rdy);
#endif
#ifdef BLOCK_SEND
      _os_ev_unlink (q->send_rdy);
#endif

      _q_free (q);
    }

  _q_unmask_irq();
  return (0);
}


/***************************************************************************/
/***************************************************************************/
/*  Send a message or messages to a queue
 *
 *  q is the queue
 *  data is a pointer to the message(s)
 *  count is the number of messages to be sent -- Count is only ONE
 *  flags is any or none of:  Q_PRIORITY, Q_BLOCK.
 *
 *  The value returned is the number of messages sucessfullt sent.
 */
int
vgl_q_send (VGL_Q *queue, void *data, int flags)
{
  int first, ev_value;
  VGL_Q_INTERNAL *q;

  q = queue->q;

  /* Block until there is space in the queue */
#ifdef BLOCK_SEND
  if ((flags&Q_BLOCK)==0)
    {
      _os_ev_read (queue->send_rdy, &ev_value);
      if (ev_value == 0)
	return(0);
    }

  _os9_ev_wait (queue->send_rdy, &ev_value, 1, q->length); 
#endif
  
  /* Mask IRQ's after blocking, otherwise the block may never exit. */
  _q_mask_irq();
  
  if (_q_stat_inline(q) < (q->length - 1))
    {
      if ((flags&Q_PRIORITY)==0)
      	{
	  /* Copy the data from the buffer into the queue */
	  _q_memcpy (((char *)queue->data) + (q->send_index * q->size),
		     data,
		     q->size);
	  
	  /* Adjust the send_index */
	  q->send_index++;	      
	  if (q->send_index >= q->length)
	    q->send_index -= q->length;
	}
      else  /* Q_PRIORITY is enabled! */
	{
	  /* Copy the data to the _FRONT_ of the buffer */
	  /* Adjust the recv index */
	  q->recv_index--;
	  if (q->recv_index < 0)
	    q->recv_index += q->length;

	  /* Copy the data in one chunk */
	  _q_memcpy (((char *)queue->data) + (q->recv_index * q->size),
		     data,
		     q->size);            	  
        } /* end if(Q_PRIORITY) else */


      /* Unmask the IRQ */
      _q_unmask_irq();
      
      /* unblock any tasks that might be waiting */
#ifdef BLOCK_RECV
      _os_ev_signal (queue->recv_rdy, &ev_value, 0);
#endif
      /* Return the number of elements received */
      return (1);
    }
  else
    {
      _q_unmask_irq();
      return (0);
    }
}


/***************************************************************************/
/***************************************************************************/
/*  Receive one or more messages from a queue.
 *
 *  q is the queue.
 *  data is the buffer to place the messages.
 *  count is the number of messages to receive.
 *  flags is one or none of:  Q_BLOCK
 *
 *  The value returned is the number of messages received.
 */
int
vgl_q_recv (VGL_Q *queue, void *data, int flags)
{
  int first, ev_value;
  VGL_Q_INTERNAL *q;

  q = queue->q;

  /* Block until the correct number of "messages" are in the queue */
#ifdef BLOCK_RECV
  if ((flags&Q_BLOCK)==0)
    {
      _os_ev_read (queue->recv_rdy, &ev_value);
      if (ev_value == 0)
	return(0);
    }
  _os9_ev_wait (queue->recv_rdy, &ev_value, 1, q->length); 
#endif
  
  /* Mask IRQ's _AFTER_ the blocking operation, otherwise it might
   * never return!
   */
  _q_mask_irq();
  
  if (_q_stat_inline(q)>0)
    {
      /* Copy the data from the queue to the specified data buffer */
      _q_memcpy (data,
		 ((char *)queue->data) + (q->recv_index * q->size),
		 q->size);
      
      /* Adjust the recv_index */
      q->recv_index++;
      if (q->recv_index >= q->length)
	q->recv_index -= q->length;

      /* Unmask the IRQ */
      _q_unmask_irq();
      
      /* unblock any tasks that might be waiting */
#ifdef BLOCK_SEND
      _os_ev_signal (queue->send_rdy, &ev_value, 0);
#endif

      /* Return the number of elements received */
      return (1); 
    }
  else
    {
      _q_mask_irq();
      return (0);
    }
}


/***************************************************************************/
/***************************************************************************/
/*  Get queue status.
 *
 *  The value returned is the number of messages in the queue.
 */
int
vgl_q_stat (VGL_Q *queue)
{
  int count;
  VGL_Q_INTERNAL *q;

  q = queue->q;
  
  _q_mask_irq();
  count = _q_stat_inline (q);
  _q_unmask_irq();
  
  return (count);                      
}


/***************************************************************************/
/***************************************************************************/
/*  Clear a queue of pending messages.
 */
void
vgl_q_clear (VGL_Q *queue)
{
  _q_mask_irq();
  _q_clear_inline (queue->q);
  _q_unmask_irq();
}


/***************************************************************************/
/***************************************************************************/
/*  Get the error status of a queue
 *
 *  q is the queue.  This value can be NULL for errors without an associated
 *  queue (I.E. Error on opening a queue).
 *
 *  Returns the error number.
 */
int
vgl_q_error (VGL_Q *queue)
{
  int err;
  VGL_Q_INTERNAL *q;

  q = queue->q;

  _q_mask_irq();
  
  if (q!=NULL)
    {
      err = q->error;
      q->error = VGL_Q_ERR_NONE;
    }
  else
    {
      err = vgl_q_global_error;
      vgl_q_global_error = VGL_Q_ERR_NONE;
    }
  
  _q_unmask_irq();
  
  return (err);
}


/***************************************************************************/
/***************************************************************************/
#endif
