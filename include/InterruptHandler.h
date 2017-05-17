/*
 * InterruptHandler.h
 *
 *  Created on: 10 мая 2017 г.
 *      Author: Abuzarov Bulat
 */

#ifndef INCLUDE_INTERRUPTHANDLER_H_
#define INCLUDE_INTERRUPTHANDLER_H_


#include <hw/inout.h>

#include <pci/pci.h>

#include <fifo.h>
#include <specific_def.h>

#include <RS_resmgr.h>

#define CPU_CLOCK_MHZ	 		125000000
#define UART_CHANNEL_COUNT		20


const struct sigevent *isr_handler (void * area, int id);
struct sigevent event;

int init(void * base_addr);
void * interrupt_thread(void * data);
void from_config(int i);


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


typedef struct channel_uart_t
{
	volatile p_uart_reg ptr_u;
	fifo_t rx_fifo;
	fifo_t tx_fifo;
	struct
	{
		int baud;
		bool mode; 		// 1 - half-duplex
		int data_bits; 	// 5, 6, 7 or 8
		int stop_bit;	// 1, 2
		int par;
	} config;
} channel_uart;

channel_uart channel[UART_CHANNEL_COUNT];


pthread_spinlock_t  fifo_spinlock[UART_CHANNEL_COUNT];
iofunc_attr_t sample_attrs[UART_CHANNEL_COUNT];

#endif /* INCLUDE_INTERRUPTHANDLER_H_ */
