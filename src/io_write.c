/*
 * io_write.c
 *
 *  Created on: 18 мая 2017 г.
 *      Author: abuzarov_bv
 */
#include <proto.h>


/*
 *  io_write
 *
 *  At this point, the client has called the library write()
 *  function, and expects that our resource manager will write
 *  the number of bytes that have been specified to the device.
 */
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
	int status;
	char *buf;

	/* Check the access permissions of the client */
	if ((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK) {
		return (status);
	}

	/* Check if pwrite() or normal write() */
	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
		return (ENOSYS);
	}

	/* First check if our message buffer was large enough
	 * to receive the whole write at once.
	 */
	if ((msg->i.nbytes <= ctp->info.msglen - ctp->offset - sizeof(msg->i))
			&& (ctp->info.msglen < ctp->msg_max_size))
	{
		/* space for NULL byte */
		buf = (char *)malloc(127 + 1);
		if (buf == NULL)
			return (ENOMEM);

		buf[msg->i.nbytes] = '\0';
		int DataSize = 128;

		char byte;
		if (msg->i.nbytes != 0)
		{
			DataSize = (DataSize >= msg->i.nbytes ? msg->i.nbytes : DataSize);
			resmgr_msgread(ctp, buf, DataSize, sizeof(msg->i));

			pthread_spin_lock(&fifo_spinlock[ctp->id]);

			fifo_put(&channel[ctp->id].tx_fifo, buf, 0, DataSize);

			for (int i = 0; i < DataSize; i++)
			{
				fifo_get(&channel[ctp->id].tx_fifo, &byte, 0, 1);
				p_uart[ctp->id]->rbr_thr_dll = byte;
			}
			p_uart[ctp->id]->ier_dlh = p_uart[ctp->id]->ier_dlh | IER_TxD_Set;

			pthread_spin_unlock(&fifo_spinlock[ctp->id]);

			_IO_SET_WRITE_NBYTES (ctp, DataSize);
			ocb->offset += DataSize;
		}
		else
			/* Set the number of bytes successfully written for
			 * the client. This information will be passed to the
			 * client by the resource manager framework upon reply.
			 * In this example, we just take the number of  bytes that
			 * were sent to us and claim we successfully wrote them.
			 */
			_IO_SET_WRITE_NBYTES (ctp, msg -> i.nbytes);
	}
	else
	{
		/* If we did not receive the whole message because the
		 * client wanted to send more than we could receive, we
		 * allocate memory for all the data and use resmgr_msgread()
		 * to read all the data at once. Although we did not receive
		 * the data completely first, because our buffer was not big
		 * enough, the data is still fully available on the client
		 * side, because its write() call blocks until we return
		 * from this callback! */
		buf = malloc(msg->i.nbytes + 1);
		resmgr_msgread(ctp, buf, msg->i.nbytes, sizeof(msg->i));
		buf[msg->i.nbytes] = '\0';
		free(buf);
	}

	/* Finally, if we received more than 0 bytes, we mark the
	 * file information for the device to be updated:
	 * modification time and change of file status time. To
	 * avoid constant update of the real file status information
	 * (which would involve overhead getting the current time), we
	 * just set these flags. The actual update is done upon
	 * closing, which is valid according to POSIX. */
	if (msg->i.nbytes > 0) {
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
	}

	return (_RESMGR_NPARTS (0));
}
