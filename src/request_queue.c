/*
 * request_queue.c
 *
 *  Created on: 22 мая 2017 г.
 *      Author: abuzarov_bv
 */
#include <request_queue.h>

int init_queue_request(callback * p_callback)
{
	callback * ptr = (callback*) malloc(sizeof(callback));
	if (ptr == NULL)
	{
		return ENOMEM;
	}
	memset(p_callback, 0, sizeof(callback));

	return EOK;
}

int add_queue_request(const callback_attr_t *arg, callback *p_callback)
{
	callback_attr_t * ptr = (callback_attr_t*) malloc(sizeof(callback_attr_t));
	if (ptr == NULL)
	{
		return ENOMEM;
	}
	memcpy(ptr, arg, sizeof(callback_attr_t));

	if (p_callback->head == NULL)
	{
		p_callback->head = ptr;
		p_callback->curr = ptr;
	}
	else
	{
		p_callback->curr->next = ptr;
		p_callback->curr = ptr;
		ptr->next = NULL; //p_callback->curr->next
	}
	p_callback->count++;
	return EOK;
}

callback_attr_t* pop_queue_request(callback *p_callback)
{
	callback_attr_t *tmp;
	if (p_callback->head)
	{
		tmp = p_callback->head;
		p_callback->head = tmp->next;
		free(tmp);
		p_callback->count--;
		return p_callback->head;
	}
	else
		return NULL;
}

int check_queue_request(callback *p_callback)
{
	if (p_callback->curr)
		return EOK;
	else
		return EFAULT;
}

int check_id_queue_request (callback *p_calback, int id)
{
	callback_attr_t *tmp;
	tmp = p_callback->head;
	if (!tmp)
		return -1;
	do
	{
		if (tmp->dev_id == id)
			return 1;
		tmp->next = tmp;
	} while (tmp->next);

	return 0;
}

void wait_queue_request(callback *p_callback)
{
	callback_attr_t *tmp;
	size_t nbytes = 0;
	int res;

	while (true)
	{
		for (int i = 0; i < UART_CHANNEL_COUNT; i++)
		{
			nbytes = fifo_count(&channel[i].rx_fifo);
			if (nbytes > 0)
			{
				res = check_id_queue_request(p_callback, i);
			}
		}
	}

	while (true)
	{
		tmp = pop_queue_request(p_callback);
		if (tmp)
		{
			MsgReply(tmp->client_id, nbytes, tmp->dev_id, nbytes);
		}
	}
}
