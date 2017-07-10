/*
 * proto.h
 *
 *  Created on: 18 мая 2017 г.
 *      Author: abuzarov_bv
 */

#ifndef INCLUDE_PROTO_H_
#define INCLUDE_PROTO_H_

//#define POLLING

#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <atomic.h>
#include <fcntl.h>

#define THREAD_POOL_PARAM_T dispatch_context_t

#include <pthread.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <sys/resmgr.h>
#include <sys/procfs.h>

#include <sys/mman.h>
#include <sys/dcmd_chr.h>

#include <sys/siginfo.h>
#include <sys/procmgr.h>
#include <malloc.h>

#include <devctl.h>
#include <termios.h>

#include <hw/inout.h>
#include <pci/pci.h>

#include <fifo.h>
#include <specific_def.h>
#include <request_queue.h>

#define CPU_CLOCK_MHZ             125000000
#define UART_CHANNEL_COUNT        32

#define MY_CMD_CODE 1

#define MY_DEVCTL_GETVAL __DIOF(_DCMD_MISC, MY_CMD_CODE + 0, int)
#define MY_DEVCTL_SETVAL __DIOT(_DCMD_MISC, MY_CMD_CODE + 1, int)
#define MY_DEVCTL_SETGET __DIOTF(_DCMD_MISC, MY_CMD_CODE + 2, union _my_devctl_msg)

#define TM_PRINTF(f_, ...) printf((f_), __VA_ARGS__)

typedef union _my_devctl_msg {
    int tx;
    int rx;
} data_t;

typedef struct dev_list
{
    uint32_t dev_open;
    uint32_t dev_res;
    uint32_t status;
    uint32_t enable;
} dev_list, *p_dev_list;

volatile dev_list dev_l;


typedef struct _uart_reg
{
    uint32_t rbr_thr_dll;
    uint32_t ier_dlh;
    uint32_t iir_fcr;
    uint32_t lcr;
    uint32_t mcr;
    uint32_t lsr;
    uint32_t msr;
    uint32_t scr;
    uint32_t afr;
    uint32_t tx_low;
} uart_reg, *p_uart_reg;

volatile p_uart_reg p_uart[UART_CHANNEL_COUNT];


typedef struct uart_setting
{
    uint32_t *IrqEnable;
    uint32_t *Mode;
    uint32_t *IrqStatus;
    uint32_t *I2cAddr;
    uint32_t *ShiftDE;
    uint32_t *ShiftTX;
    uint32_t *ShiftRX;
} uart_setting, *p_uart_setting;

static volatile uart_setting uart_set;

#include <request_struct.h>

typedef struct channel_uart_t
{
    volatile p_uart_reg ptr_u;
    fifo_t rx_fifo;
    fifo_t tx_fifo;
    struct
    {
        int baud;
        bool mode;         // 1 - half-duplex
        int data_bits;     // 5, 6, 7 or 8
        int stop_bit;    // 1, 2
        int par;
    } config;
    callback p_callback;
} channel_uart;


channel_uart channel[UART_CHANNEL_COUNT];

pthread_spinlock_t  fifo_spinlock[UART_CHANNEL_COUNT];
iofunc_attr_t sample_attrs[UART_CHANNEL_COUNT];

int io_open   (resmgr_context_t *ctp, io_open_t  *msg, RESMGR_HANDLE_T *handle, void *extra);
int io_close (resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb);
int io_read   (resmgr_context_t *ctp, io_read_t  *msg, RESMGR_OCB_T *ocb);
int io_write  (resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
int io_devctl (resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
int io_unblock(resmgr_context_t *ctp, io_pulse_t *msg, RESMGR_OCB_T *ocb);

void options     (int argc, char **argv);

int pci_init            (void);
void port_init          (void * base_addr);
void * interrupt_thread (void * data);
void from_config        (int i);

const struct sigevent *isr_handler (void * area, int id);

void handler       (void * args);
void sighandler    (int signum, siginfo_t * siginfo, void * context);

void check_rx_and_reply(channel_uart *channel, unsigned char *buf);
void check_tx_and_reply(channel_uart *channel, unsigned char *buf);

#endif /* INCLUDE_PROTO_H_ */
