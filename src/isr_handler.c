/*
 * InterruptHandler.c
 *
 *  Created on: 10 мая 2017 г.
 *      Author: abuzarov_bv
 */

#include <proto.h>


struct sigevent event;

void * interrupt_thread(void * data)
{
	int id, err;
	id = pci_init();

	memset(&event, 0, sizeof(event));
	event.sigev_notify = SIGEV_INTR;

	//  x8 x6 x7 x5 сюда будут передаваться параметры какие порты необходимо открыть
	dev_l.dev_open = (1 << 17) | (1 << 18) | (1 << 12) | (1 << 13);
	//    for (int i = 0; i < UART_CHANNEL_COUNT; i++) {
	//    	((1 << i) & dev_list) ? printf("%d:1\n",i) :printf("%d:0\n",i);
	//    }

	if ((err = InterruptUnmask(17, id)) == -1)
	{
		perror("Can`t interrupt unmask\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < UART_CHANNEL_COUNT; i++)
	{
		memset(&fifo_spinlock[i], 0, sizeof(fifo_spinlock[i]));
	}

	uint8_t byte;

//	*(uart_set.IrqEnable) = (1 << 2) | (1 << 1) | (1 << 7) | (1 << 6);
	*(uart_set.IrqEnable) |= (0xFFFFFFFF);
	while (1)
	{
		InterruptWait(0, NULL);
		for (int i = 0; i < UART_CHANNEL_COUNT; i++)
		{
			pthread_spin_lock(&fifo_spinlock[i]);
			// Check open device
			if ((1 << i) & dev_l.dev_open)
			{
				//if Rx
				while (LSR_DR_Get(p_uart[i]->lsr))
				{
					byte = (uint8_t) p_uart[i]->rbr_thr_dll;
					if (byte != '\0') fifo_put(&channel[i].rx_fifo, &byte, 0, 1);
//					printf("ch%d:0x%x - %c \n", i, byte, byte);
				}
				//if Tx
				uint32_t lsr = p_uart[i]->lsr;
				if (IIR_Get(p_uart[i]->iir_fcr) == TXFIFO_EMPTY)
				{
					if (LSR_THRE_Get(lsr) || LSR_TEMPT_Get(lsr))
					{
						p_uart[i]->ier_dlh = IER_RxD_Set; // disable interrupt from txfifo until we write down there
					}
				}
				sample_attrs[i].nbytes = fifo_count(&channel[i].rx_fifo);
//				*(uart_set.IrqEnable) |= (1 << (19 - i));
			}
			pthread_spin_unlock(&fifo_spinlock[i]);
		}
		*(uart_set.IrqEnable) &= ~(0xFFFFFFFF);
		*(uart_set.IrqEnable) |= (0xFFFFFFFF);
		InterruptUnmask(17, id);
	}
}

void port_init(void * base_addr)
{
	uart_set.IrqEnable = (uint32_t *) (base_addr + 0x2000120);
	uart_set.IrqStatus = (uint32_t *) (base_addr + 0x2000100);
	uart_set.Mode = (uint32_t *) (base_addr + 0x2000060);
	*(uart_set.Mode) = 0xFFFFFFFF;
	*(uart_set.IrqEnable) &= ~(0xFFFFFFFF);

	uart_set.I2cAddr = (uint32_t *) (base_addr + 0x2000400);
	uart_set.ShiftDE = (uint32_t *) (base_addr + 0x2000200);
	uart_set.ShiftTX = (uint32_t *) (base_addr + 0x2000210);
	uart_set.ShiftRX = (uint32_t *) (base_addr + 0x2000220);

	for (int i = 0; i < UART_CHANNEL_COUNT; i++)
	{
		uint32_t LCR = 0;
		LCR = p_uart[i]->lcr;
		p_uart[i]->lcr = LCR | LCR_DLAB_Set;
		uint32_t DIV = CPU_CLOCK_MHZ / (B115200 * 16);
		p_uart[i]->rbr_thr_dll = DLL_Set(DIV);
		p_uart[i]->ier_dlh = DLH_Set(DIV >> 8);
		LCR = p_uart[i]->lcr;
		LCR &= ~LCR_DLAB_Set;
		p_uart[i]->lcr = LCR | LCR_SDB_Set(LCR_SDB_MODE_08);
//		p_uart[i]->mcr = 1 << 4;
		p_uart[i]->mcr = 0;
		p_uart[i]->iir_fcr = FCR_FIFO_ENABLE_Set | FCR_RESETRF_Set
				| FCR_RESETTF_Set;
		p_uart[i]->ier_dlh = IER_RxD_Set;
		channel[i].config.mode = true;
		channel[i].config.baud = B115200;
		channel[i].config.data_bits = MODE_08_TO_CS(LCR_SDB_MODE_08);
	}
}

const struct sigevent *isr_handler(void * area, int id)
{
	((dev_list *) area)->status = *(uart_set.IrqStatus);
	((dev_list *) area)->enable = *(uart_set.IrqEnable);
	if (*(uart_set.IrqStatus) == 0)
		return (NULL);
	return (&event);
}

void from_config(int i)
{
	int Baud;
	Baud = channel[i].config.baud;
	int Mode;
	Mode = *(uart_set.Mode);

	if (channel[i].config.mode)
	{
		SetNbit(Mode, (19 - i));
	}
	else
	{
		ClearBit(Mode, (19 - i));
	}
	*(uart_set.Mode) = Mode;

	Baud = channel[i].config.baud;

	uint32_t LCR = 0;
	LCR = p_uart[i]->lcr;
	p_uart[i]->lcr = LCR | LCR_DLAB_Set;
	uint32_t DIV = CPU_CLOCK_MHZ / (Baud * 16);
	p_uart[i]->rbr_thr_dll = DLL_Set(DIV);
	p_uart[i]->ier_dlh = DLH_Set(DIV >> 8);
	LCR = p_uart[i]->lcr;
	LCR &= ~LCR_DLAB_Set;
	p_uart[i]->lcr = LCR | LCR_SDB_Set(MODE_CS_TO_(channel[i].config.data_bits));
	p_uart[i]->mcr = 0;
	p_uart[i]->iir_fcr = FCR_FIFO_ENABLE_Set | FCR_RESETRF_Set
			| FCR_RESETTF_Set;
	p_uart[i]->ier_dlh = IER_RxD_Set;
}
