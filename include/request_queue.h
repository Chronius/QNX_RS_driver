/*
 * request_queue.h
 *
 *  Created on: 22 мая 2017 г.
 *      Author: abuzarov_bv
 */

#ifndef INCLUDE_REQUEST_QUEUE_H_
#define INCLUDE_REQUEST_QUEUE_H_

#include <proto.h>

typedef struct callback_t
{
	typedef struct callback_attr_t
	/*
	 * client_id - ID client for MsgReply
	 * nbytes 	 - lenght buffer client
	 * dev_id	 - number of channel that need reply msg
	 */
	{
		int client_id;
		int nbytes;
		int dev_id;
		struct callback_attr_t *next;
	} callback_attr;
	int count;
	struct callback_attr *head;
	struct callback_attr *curr;
} callback;
callback *p_callback;

int init_queue_request			(callback * p_callback);
int add_queue_request			(const callback *arg, callback *p_callback);
int check_id_queue_request 		(callback *p_calback, int id);
void wait_queue_request			(callback *p_callback);
void check_queue_request		(callback *arg);
callback_attr* pop_queue_request(callback *p_callback);

#endif /* INCLUDE_REQUEST_QUEUE_H_ */
