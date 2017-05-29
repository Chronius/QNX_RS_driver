/*
 * request_struct.h
 *
 *  Created on: 24 мая 2017 г.
 *      Author: abuzarov_bv
 */

#ifndef INCLUDE_REQUEST_STRUCT_H_
#define INCLUDE_REQUEST_STRUCT_H_

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
//    callback_attr_t *queue[20];
//    callback_attr_t *queue_old[20];
} callback;


#endif /* INCLUDE_REQUEST_STRUCT_H_ */
