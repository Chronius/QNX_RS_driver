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
	size_t nbytes = 0;
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

	if (msg->i.nbytes <= 0) {
        _IO_SET_READ_NBYTES(ctp, 0);
        return _RESMGR_NPARTS(0);
    }
	/*	ocb->attr указывает на sample_attrs[i].nbytes
	 *  в контексте данной сессии
	 */

	unsigned char *buffer = NULL;
	if ((buffer = malloc(ocb->attr->nbytes + 1)) == NULL)
		return (ENOMEM);
	memset(buffer, 0, sizeof(ocb->attr->nbytes + 1));
	ocb->attr->nbytes = fifo_count(&channel[ctp->id].rx_fifo);
	nbytes = min(_IO_READ_GET_NBYTES(msg), ocb->attr->nbytes);

	/*
	 *	Test function Прерывание cat по Ctrl-C ставиться в очередь
	 *	и срабатывает только по прибытию нового символа,
	 *	MsgReply - разблокирует клиента только если буфер не пустой
	 *	Прерывание по Ctrl-D
	 */

	unsigned char *buf = NULL;

#ifndef Debug

	nbytes = fifo_count(&channel[ctp->id].rx_fifo);
	if (nbytes > 0)
	{
		buf = malloc(nbytes + 1);
		if (buf == NULL)
			return (ENOMEM);

		pthread_spin_lock(&fifo_spinlock[ctp->id]);
		fifo_get(&channel[ctp->id].rx_fifo, buf, 0, nbytes);
		MsgReply(ctp->rcvid, nbytes, buf, nbytes);
		pthread_spin_unlock(&fifo_spinlock[ctp->id]);

		ocb->attr->flags |=
		IOFUNC_ATTR_ATIME | IOFUNC_ATTR_DIRTY_TIME;

		free(buf);
		free(buffer);
		return _RESMGR_NOREPLY;
	}
#else
	/*
	 *	Cat реагирует на сигнал Ctrl+C, но при
	 *	выводе в файл терминальные нули '\0'
	 *	также будут записаны
	 *
	 *	!!! забивает память
	 */
	if (nbytes > 0)
	{
		pthread_spin_lock(&fifo_spinlock[ctp->id]);
		fifo_get(&channel[ctp->id].rx_fifo, buffer, 0, nbytes);
		pthread_spin_unlock(&fifo_spinlock[ctp->id]);

		/* set up the return data IOV */
		_IO_SET_READ_NBYTES (ctp, nbytes);
		MsgReply(ctp->rcvid, nbytes, buffer, nbytes);
		memset(buffer, 0, sizeof(ocb->attr->nbytes + 1));
		ocb->attr->flags |=
		IOFUNC_ATTR_ATIME | IOFUNC_ATTR_DIRTY_TIME;
		/*
		 * advance the offset by the number of bytes
		 * returned to the client.
		 */
		ocb->offset += nbytes;
	}
	else
	{
//		nparts = 0;
//		char c = '\0';
//		MsgReply(ctp->rcvid, 1, &c, 1);
//		_IO_SET_READ_NBYTES (ctp, 0);
		 _IO_SET_READ_NBYTES (ctp, 0);
		 MsgReply(ctp->rcvid, 1, buffer, 0);
	}

#endif
	/* mark the access time as invalid (we just accessed it) */
	if (nbytes > 0)
	{
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;
	}
	free((void*)buffer);
	free((void*)buf);
	MsgReply(ctp->rcvid, EOK, NULL, 0);
	return _RESMGR_NOREPLY;
}
