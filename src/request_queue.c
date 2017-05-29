/*
 * request_queue.c
 *
 *  Created on: 22 мая 2017 г.
 *      Author: abuzarov_bv
 */
#include <request_queue.h>

#ifdef DEBUG
int init_queue_request(callback * p_callback)
{
    callback * tmp = (callback*) malloc(sizeof(callback));
    if (tmp == NULL)
    {
        return ENOMEM;
    }
    memset(tmp, 0, sizeof(callback));
    p_callback = tmp;

    return EOK;
}

callback_attr_t* peek_request(callback *p_callback)
{
    if (p_callback->head)
    {
        return p_callback->head;
    }
    return NULL ;
}

int enqueue_request(const callback_attr_t *attr, callback *p_callback)
{
    callback_attr_t * tmp = (callback_attr_t*) malloc(sizeof(callback_attr_t));
    if (tmp == NULL)
    {
        return ENOMEM;
    }
    memcpy(tmp, attr, sizeof(callback_attr_t));

	for (int i = 0; i < 20; i++)
	{
		if (p_callback->queue[i] == NULL)
		{
			p_callback->queue[i] = tmp;
			p_callback->length++;
			return EOK;
		}
	}
	return -1;
}

int dequeue_request(callback *p_callback, callback_attr_t *buf, int n)
{
	memcpy(p_callback->queue_old, p_callback->queue, sizeof(p_callback->queue));
	for (int i = 0; i < 20; i++)
	{
		if (n == i)
		{
			buf = p_callback->queue_old[i];
			return 1;
			p_callback->queue_old[i] = NULL;
			p_callback->length--;
		}
	}
	int count = 0;
	memset(p_callback->queue, 0, sizeof(p_callback->queue));
	for (int i = 0; i < 20; i++)
	{
		if (p_callback->queue_old[i] != NULL)
		{
			p_callback->queue[count++] = p_callback->queue_old[i];
		}
	}
	return EOK;
}

int check_queue_request(callback *p_callback)
{
    if (p_callback->curr)
        return EOK;
    else
        return EFAULT;
}

int check_id_queue_request(callback *p_callback, int id)
{
    for (int i = 0; i < 20; i++)
    {
    	if (p_callback->queue[i] != NULL)
    		if (p_callback->queue[i]->dev_id == id)
    			return 1;
    }
    return 0;
}

void * wait_queue_request(void *arg)
{
    callback *p_callback = (callback *) arg;
    init_queue_request(p_callback);
    size_t nbytes = 0;
    int res;
    unsigned char *buf = NULL;
    callback_attr_t *tmp = NULL;
    callback_attr_t foo;
    while (true)
    {
        if (p_callback->length)
        {
            {
                //need lock from isr handler
                for (int i = 0; i < UART_CHANNEL_COUNT; i++)
                {
                	pthread_spin_lock(&fifo_spinlock[i]);
                    nbytes = fifo_count(&channel[i].rx_fifo);
                    if (nbytes > 0)
                    {
                        //block
                        buf = malloc(nbytes + 1);
                        if (buf == NULL)
                        {
                            exit(ENOMEM);
                        }
//                        res = check_id_queue_request(p_callback, i);
                        for (int j = 0; j < 20; j++)
						{
							if (p_callback->queue[j] != NULL)
								if (p_callback->queue[j]->dev_id == i)
								{
									dequeue_request(p_callback, tmp, j);
									fifo_get(&channel[p_callback->queue[j]->dev_id].rx_fifo, buf, 0, nbytes);
									MsgReply(p_callback->queue[j]->client_id, nbytes, buf, nbytes);
//									dequeue_request(p_callback, NULL, j);
									free(tmp);
								}
						}
                        free(buf);
                    }
                    pthread_spin_unlock(&fifo_spinlock[i]);
                }
            }
        }
    }
}

#else
int init_queue_request(callback * p_callback)
{
//    callback * tmp = (callback*) malloc(sizeof(callback));
//    if (tmp == NULL)
//    {
//        return ENOMEM;
//    }
    memset(p_callback, 0, sizeof(callback));
//    p_callback = tmp;

    return EOK;
}

/*
 * returns a pointer to the head of the queue
*/
callback_attr_t* peek_request(callback *p_callback)
{
    if (p_callback->head)
    {
        return p_callback->head;
    }
    return NULL ;
}

int enqueue_request(const callback_attr_t *attr, callback *p_callback)
{
    callback_attr_t * tmp = (callback_attr_t*) malloc(sizeof(callback_attr_t));
    if (tmp == NULL)
    {
        return ENOMEM;
    }
    memcpy(tmp, attr, sizeof(callback_attr_t));

    if (p_callback->head == NULL)
    {
        p_callback->head = tmp;
        p_callback->curr = tmp;
        tmp->next = NULL;
    }
    else
    {
        p_callback->curr->next = tmp;
        p_callback->curr = tmp;
        tmp->next = NULL; //p_callback->curr->next
    }
    p_callback->length++;
    return EOK;
}

int dequeue_request(callback *p_callback, callback_attr_t *buf)
{
    if (p_callback->head)
    {
        if (!buf)
        {
            buf = p_callback->head;
            p_callback->head = buf->next;
            free(buf);
            p_callback->length--;
            return EOK;
        }
        memcpy(buf, p_callback->head, sizeof(callback_attr_t));
        buf = p_callback->head;
        p_callback->head = buf->next;
        free(buf);
        p_callback->length--;
        return EOK;
    }
    else
        return -1 ;
}

int check_queue_request(callback *p_callback)
{
    if (p_callback->curr)
        return EOK;
    else
        return EFAULT;
}

int check_id_queue_request(callback *p_callback, int id)
{
    callback_attr_t *tmp;
    tmp = p_callback->head;
    if (!tmp)
        return -1;
    if (tmp->dev_id == id)
		return 1;
    return 0;
    do
    {
        if (tmp->dev_id == id)
            return 1;
        tmp = tmp->next;
    } while (tmp);

    return 0;
}

int len_queue_request(callback *p_callback)
{
	if (p_callback->head)
		return p_callback->length;
	return 0;
}

void check_rx_and_reply(channel_uart *channel, unsigned char *buf)
{
	int len_request = 0;
	int nbytes = 0;
	len_request = len_queue_request(&channel->p_callback);
	nbytes = fifo_count(&channel->rx_fifo);

	if ((len_request > 0) && (nbytes > 0))
	{
		callback_attr_t *tmp = NULL;
		tmp = peek_request(&channel->p_callback);
//		pthread_spin_lock(&fifo_spinlock[tmp->dev_id]);

		fifo_get(&channel->rx_fifo, buf, 0, nbytes);
		MsgReply(tmp->client_id, nbytes, buf, nbytes);
		dequeue_request(&channel->p_callback, NULL);
//		pthread_spin_unlock(&fifo_spinlock[tmp->dev_id]);
	}
}

void check_tx_and_reply(channel_uart *channel, unsigned char *buf)
{
	int len_request = 0;
	int nbytes, id;
	const int DataSize = 128;
	unsigned char buffer[DataSize];

	len_request = len_queue_request(&channel->p_callback);
//	nbytes = fifo_count(&channel->tx_fifo);
//	nbytes = (DataSize >= nbytes ? nbytes : DataSize);

	callback_attr_t *tmp = NULL;
	tmp = peek_request(&channel->p_callback);
	if (tmp)
	{
		id = tmp->dev_id;
		nbytes = tmp->nbytes;
		nbytes = (DataSize >= nbytes ? nbytes : DataSize);
		MsgRead(tmp->client_id, &buffer, nbytes, tmp->offset);
		for (int i = 0; i < nbytes; i++)
		{
			p_uart[id]->rbr_thr_dll = buffer[i];
		}
//		if ((len_request > 0) && (nbytes > 0))
//		{
//			fifo_get(&channel->tx_fifo, buf, 0, nbytes);
//
//			for (int i = 0; i < nbytes; i++)
//			{
//				p_uart[id]->rbr_thr_dll = buf[i];
//			}
//		}

		MsgReply(tmp->client_id, nbytes, NULL, 0);
//		MsgReply(tmp->client_id, 1, NULL, 0);
		tmp->nbytes -= nbytes;
		if (nbytes == 0)
		{
//			MsgReply(tmp->client_id, 0, NULL, nbytes);
			dequeue_request(&channel->p_callback, NULL);
			p_uart[id]->ier_dlh = IER_RxD_Set;
		}
//		else
//		{
//			p_uart[id]->ier_dlh = p_uart[id]->ier_dlh | IER_TxD_Set;
//		}
		p_uart[id]->ier_dlh = p_uart[id]->ier_dlh | IER_TxD_Set;
	}
}
void * wait_foo(void *arg)
{
//	callback *p_callback = (callback *) arg;
	int nbytes = 0;
	int len_request = 0;
	int res;
	unsigned char *buf = NULL;
	callback *p_callback = NULL;
	callback_attr_t *tmp = NULL;
	buf = malloc(FIFO_LENGTH);

	while (true)
	{
//		if (p_callback->length)
		{
			for (int i = 0; i < UART_CHANNEL_COUNT; i++)
			{
				pthread_spin_lock(&fifo_spinlock[i]);
				p_callback = &channel[i].p_callback;
				len_request = len_queue_request(p_callback);
				nbytes = fifo_count(&channel[i].rx_fifo);
				if ((len_request > 0) && (nbytes > 0))
				{
					tmp = peek_request(p_callback);
					fifo_get(&channel[i].rx_fifo, buf, 0, nbytes);
					MsgReply(tmp->client_id, nbytes, buf, nbytes);
					dequeue_request(p_callback, NULL);
				}
				pthread_spin_unlock(&fifo_spinlock[i]);
			}
		}
	}

}

void * wait_queue_request(void *arg)
{
    callback *p_callback = (callback *) arg;
    int nbytes = 0;
    int res;
    unsigned char *buf = NULL;
    callback_attr_t *tmp = NULL;

    while (true)
    {
        if (p_callback->length)
        {
            tmp = peek_request(p_callback);
            if (tmp)
            {
                //need lock from isr handler
            	pthread_spin_lock(&fifo_spinlock[tmp->dev_id]);
                for (int i = 0; i < UART_CHANNEL_COUNT; i++)
                {
                    nbytes = fifo_count(&channel[i].rx_fifo);
                    if (nbytes > 0)
                    {
                        //block
                        buf = malloc(nbytes + 1);
                        if (buf == NULL)
                        {
                        	pthread_spin_unlock(&fifo_spinlock[tmp->dev_id]);
                            exit(ENOMEM);
                        }
                        res = check_id_queue_request(p_callback, i);
                        if (res == 1)
                        {
                            fifo_get(&channel[tmp->dev_id].rx_fifo, buf, 0, nbytes);
                            MsgReply(tmp->client_id, nbytes, buf, nbytes);
                            dequeue_request(p_callback, NULL);
                        }
                        free(buf);
                    }
                }
                pthread_spin_unlock(&fifo_spinlock[tmp->dev_id]);
            }
        }
    }
}
#endif
