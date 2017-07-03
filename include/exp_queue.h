/*
 * request_queue.h
 *
 *  Created on: 22 мая 2017 г.
 *      Author: abuzarov_bv
 */

#ifndef INCLUDE_EXP_QUEUE_H_
#define INCLUDE_EXP_QUEUE_H_

#include <proto.h>
 
typedef struct callback_attr_t
/*
 * client_id - ID client for MsgReply
 * nbytes    - lenght buffer client
 * dev_id    - number of channel that need reply msg
 */
{
    int client_id;
    int nbytes;
    int dev_id;
    int offset;
    struct callback_attr_t *next;
} callback_attr_t;

typedef struct callback_t
{

    callback_attr_t callback_attr;
    int length;
    struct callback_attr_t *head;
    struct callback_attr_t *curr;

    int queue_begin, queue_end;
    callback_attr_t callback_queue[QUEUE_MAX_SIZE];
} callback;

int init_queue_request          (callback * p_callback);
int check_id_queue_request      (callback *p_callback, int id);
int check_queue_request         (callback *p_callback);
int enqueue_request             (const callback_attr_t *attr, callback *p_callback);
int dequeue_request             (callback *p_callback);
int len_queue_request			(callback *p_callback);

void * wait_queue_request       (void *p_callback);
void * wait_foo					(void *arg);
callback_attr_t* peek_request   (callback *p_callback);
#endif
#endif /* INCLUDE_REQUEST_QUEUE_H_ */
