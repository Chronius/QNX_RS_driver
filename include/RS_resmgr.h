/*
 * RS_resmgr.h
 *
 *  Created on: 10 мая 2017 г.
 *      Author: abuzarov_bv
 */

#ifndef INCLUDE_RS_RESMGR_H_
#define INCLUDE_RS_RESMGR_H_



#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define THREAD_POOL_PARAM_T dispatch_context_t

#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <sys/resmgr.h>

#include <sys/siginfo.h>
#include <sys/procmgr.h>
#include <malloc.h>

#include <sys/mman.h>
#include <sys/dcmd_chr.h>

#include <devctl.h>
#include <termios.h>

typedef union _my_devctl_msg {
	int tx;
	int rx;
} data_t;

#define MY_CMD_CODE 1

#define MY_DEVCTL_GETVAL __DIOF(_DCMD_MISC, MY_CMD_CODE + 0, int)
#define MY_DEVCTL_SETVAL __DIOT(_DCMD_MISC, MY_CMD_CODE + 1, int)
#define MY_DEVCTL_SETGET __DIOTF(_DCMD_MISC, MY_CMD_CODE + 2, union _my_devctl_msg)

#endif /* INCLUDE_RS_RESMGR_H_ */
