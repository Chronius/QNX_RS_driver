/*
 * request_queue.h
 *
 *  Created on: 22 мая 2017 г.
 *      Author: abuzarov_bv
 */

#ifndef INCLUDE_REQUEST_QUEUE_H_
#define INCLUDE_REQUEST_QUEUE_H_

#include <proto.h>
#include <request_struct.h>


//callback *p_callback;int init_queue_request          (callback * p_callback);
#ifdef DEBUG
int check_id_queue_request      (callback *p_callback, int id);
int check_queue_request         (callback *p_callback);
int enqueue_request             (const callback_attr_t *attr, callback *p_callback);
void * wait_queue_request       (void *p_callback);
callback_attr_t* peek_request   (callback *p_callback);
int dequeue_request(callback *p_callback, callback_attr_t *buf, int n);
#else

int init_queue_request          (callback * p_callback);
int check_id_queue_request      (callback *p_callback, int id);
int check_queue_request         (callback *p_callback);
int enqueue_request             (const callback_attr_t *attr, callback *p_callback);
int dequeue_request             (callback *p_callback, callback_attr_t *buf);
int len_queue_request			(callback *p_callback);

void * wait_queue_request       (void *p_callback);
void * wait_foo					(void *arg);
callback_attr_t* peek_request   (callback *p_callback);
#endif
#endif /* INCLUDE_REQUEST_QUEUE_H_ */
