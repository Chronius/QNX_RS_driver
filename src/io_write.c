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
	int off;
	int start_data_offset;
	struct _xtype_offset *xoffset;

	/* Check the access permissions of the client */
	if ((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK)
	{
		return (status);
	}

	/* Check if pwrite() or normal write() */
	int xtype = msg->i.xtype & _IO_XTYPE_MASK;
	if (xtype == _IO_XTYPE_OFFSET)
	{
		xoffset = (struct _xtype_offset*) (&msg->i + 1);
		start_data_offset = sizeof(msg->i) + sizeof(*xoffset);
		off = xoffset->offset;
	}
	else if (xtype == _IO_XTYPE_NONE)
	{
		off = ocb->offset;
		start_data_offset = sizeof(msg->i);
	}
	else
	{
		// Неизвестный тип; игнорировать
		return (ENOSYS);
	}

	char *buffer;
	int nbytes;
	int DataSize = 128;
	unsigned char Data[128] = {};
	int offset = 0;
	nbytes = msg->i.nbytes;
	if ((buffer = malloc(nbytes)) == NULL)
	{
		return (ENOMEM);
	}

//	MsgRead()
	if (resmgr_msgread(ctp, buffer, nbytes, start_data_offset) == -1)
	{
		free(buffer);
		return (errno);
	}

#ifdef DEBUG
	if (nbytes)
	{
		callback_attr_t attr;
		attr.client_id = ctp->rcvid;
		attr.nbytes = nbytes;
		attr.offset = start_data_offset;
		attr.dev_id = ctp->id;
		pthread_spin_lock(&fifo_spinlock[ctp->id]);
		enqueue_request(&attr, &channel[ctp->id].p_callback);
		p_uart[ctp->id]->ier_dlh = p_uart[ctp->id]->ier_dlh | IER_TxD_Set;
		pthread_spin_unlock(&fifo_spinlock[ctp->id]);


		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
	}
	else
	{
		MsgReply(ctp->rcvid, EOK, NULL, 0);
	}

	free(buffer);
	return _RESMGR_NOREPLY;
#else
	if (nbytes != 0)
	{
		uint32_t lsr;
		DataSize = (DataSize >= nbytes ? nbytes : DataSize);
		while (nbytes > 0)
		{
			lsr = p_uart[ctp->id]->lsr;
			if (LSR_THRE_Get(lsr) || LSR_TEMPT_Get(lsr))
			{
				pthread_spin_lock(&fifo_spinlock[ctp->id]);
				fifo_put(&channel[ctp->id].tx_fifo, buffer, offset, DataSize);
				fifo_get(&channel[ctp->id].tx_fifo, Data, 0, DataSize);

				for (int i = 0; i < DataSize; i++)
				{
					p_uart[ctp->id]->rbr_thr_dll = Data[i];
				}
//				p_uart[ctp->id]->ier_dlh = p_uart[ctp->id]->ier_dlh | IER_TxD_Set;
				pthread_spin_unlock(&fifo_spinlock[ctp->id]);

				nbytes -= DataSize;
				offset += DataSize;
			}
			else
				continue;
		}

		_IO_SET_WRITE_NBYTES(ctp, DataSize);

		if (xtype == _IO_XTYPE_NONE)
		{
			ocb->offset += nbytes;
		}
	}
	else
	{
		/* Set the number of bytes successfully written for
		 * the client. This information will be passed to the
		 * client by the resource manager framework upon reply.
		 * In this example, we just take the number of  bytes that
		 * were sent to us and claim we successfully wrote them.
		 */
		_IO_SET_WRITE_NBYTES(ctp, nbytes);
	}
#endif
	free(buffer);

	if (nbytes)
	{
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	}

	return (_RESMGR_NPARTS (0));
}
