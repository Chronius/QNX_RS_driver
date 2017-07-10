/*
 * read.c
 *
 *  Created on: 18 мая 2017 г.
 *      Author: abuzarov_bv
 */
#include <proto.h>

/*
 *  io_read
 *
 * The message that we received can be accessed via the
 * pointer *msg. A pointer to the OCB that belongs to this
 * read is the *ocb. The *ctp pointer points to a context
 * structure that is used by the resource manager framework
 * to determine whom to reply to, and more. */

int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
	int status;
	int nonblock;
	/* Here we verify if the client has the access
	 * rights needed to read from our device */
	if ((status = iofunc_read_verify(ctp, msg, ocb, &nonblock)) != EOK)
	{
		return (status);
	}
	/* Client does`n want blocking */
	if (nonblock)
	{
		return (EAGAIN);
	}
	/* We check if our read callback was called because of
	 * a pread() or a normal read() call. If pread(), we return
	 * with an error code indicating that we don't support it.*/
	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
	{
		return (ENOSYS);
	}

	if (msg->i.nbytes <= 0)
	{
		_IO_SET_READ_NBYTES(ctp, 0);
		return _RESMGR_NPARTS(0);
	}

	callback_attr_t attr;
	attr.client_id = ctp->rcvid;
	attr.nbytes = _IO_READ_GET_NBYTES(msg);
	attr.dev_id = ctp->id;

	pthread_spin_lock(&fifo_spinlock[ctp->id]);
	enqueue_request(&attr, &channel[ctp->id].p_callback);
	pthread_spin_unlock(&fifo_spinlock[ctp->id]);

	return _RESMGR_NOREPLY ;

}
