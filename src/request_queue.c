/*
 * request_queue.c
 *
 *  Created on: 22 мая 2017 г.
 *      Author: abuzarov_bv
 */
#include <request_queue.h>

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

void * wait_queue_request(void *arg)
{
    callback *p_callback = (callback *) arg;
    size_t nbytes = 0;
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
