#include <exp_queue.h>

#define QUEUE_MAX_SIZE  20


int init_queue_request(callback * p_callback)
{
    memset(p_callback, 0, sizeof(callback));

    return EOK;
}

/*
 * returns a pointer to the head of the queue
*/
callback_attr_t* peek_request(callback *p_callback)
{
    if (p_callback->callback_queue[p_callback->queue_begin].status)
    {
        return p_callback->callback_queue[p_callback->queue_begin];
    }
    return NULL;
}

int enqueue_request(const callback_attr_t *attr, callback *p_callback)
{
    int end = p_callback->queue_end;
    if (end == QUEUE_MAX_SIZE)
    {
        end = 0;
        memcpy(p_callback->callback_queue[end], attr, sizeof(callback_attr_t));
        p_callback->callback_queue[end++].status = true;
    }
    else
    {
        memcpy(p_callback->callback_queue[end], attr, sizeof(callback_attr_t));
        p_callback->callback_queue[end++].status = true;
    }

    p_callback->queue_end = end;
    p_callback->length++;
    return EOK;
}

int dequeue_request(callback *p_callback)
{
    int head = p_callback->queue_begin;

    if (p_callback->callback_queue[head].status)
    {
        p_callback->callback_queue[head].status = false;

        if (head == QUEUE_MAX_SIZE)
        {
            p_callback->queue_begin == 0;   
        }
        else
        {
            p_callback->queue_begin++;
        }

        p_callback->length--;
        return EOK;
    }
    else
        return -1 ;
}


int len_queue_request(callback *p_callback)
{
    return p_callback->length;
}

void check_rx_and_reply(channel_uart *channel, unsigned char *buf)
{
    int len_request = 0;
    int nbytes = 0;
    len_request = p_callback->length;
    nbytes = fifo_count(&channel->rx_fifo);

    if ((len_request > 0) && (nbytes > 0))
    {
        callback_attr_t *tmp = NULL;
        tmp = peek_request(&channel->p_callback);
//        pthread_spin_lock(&fifo_spinlock[tmp->dev_id]);

        fifo_get(&channel->rx_fifo, buf, 0, nbytes);
        MsgReply(tmp->client_id, nbytes, buf, nbytes);
        dequeue_request(&channel->p_callback);
//        pthread_spin_unlock(&fifo_spinlock[tmp->dev_id]);
    }
}

void check_tx_and_reply(channel_uart *channel, unsigned char *buf)
{
    int len_request = 0;
    int nbytes, id;
    const int DataSize = 128;
    unsigned char buffer[DataSize];

    len_request = p_callback->length;

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

        MsgReply(tmp->client_id, nbytes, NULL, 0);
        tmp->nbytes -= nbytes;
        if (nbytes == 0)
        {
//            MsgReply(tmp->client_id, 0, NULL, nbytes);
            dequeue_request(&channel->p_callback);
            p_uart[id]->ier_dlh = IER_RxD_Set;
        }
        p_uart[id]->ier_dlh = p_uart[id]->ier_dlh | IER_TxD_Set;
    }
}