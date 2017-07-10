/*
 * pci_init.c
 *
 *  Created on: 18 мая 2017 г.
 *      Author: abuzarov_bv
 */

#include <proto.h>

int pci_init()
{
	void *ptr, *ptr_to_set;
	pci_irq_t irq[2];
	int_t nirq = NELEMENTS(irq);
	pci_bdf_t dev;
	pci_cs_t cs;
	pci_ba_t ba[4];
	int_t nba = NELEMENTS(ba);

	pci_devhdl_t handler = NULL; // void * handler
	pci_vid_t VendorId = 0x1172;
	pci_did_t DeviceId = 0xe001;
	pci_err_t r;
	int id;

	dev = pci_device_find(0, VendorId, DeviceId, PCI_CCODE_ANY);

	printf("Device found ");
	printf("B%u:D%u:F%u\n", PCI_BUS(dev), PCI_DEV(dev), PCI_FUNC(dev));
	cs = pci_device_chassis_slot(dev);
	printf("CHASSIS:%u:SLOT:%u\n", PCI_CHASSIS(cs), PCI_SLOT(cs));

	handler = pci_device_attach(dev, pci_attachFlags_OWNER, &r);
	puts("Device attach");
	printf("Handler: 0x%x\n", handler);

	if (handler == NULL)
	{
		perror("Can`t device attach");
		exit(EXIT_FAILURE);
	}

	r = pci_device_read_ba(handler, &nba, ba, pci_reqType_e_UNSPECIFIED);
	if ((r == PCI_ERR_OK) && (nba > 0))
	{
		uint_t i;
		printf("Mem base list\n");
		for (i = 0; i < nba; i++)
		{
			printf("BAR %d\tbase address: 0x%x\n", ba[i].bar_num, ba[i].addr);
			if (ba[i].size == 0x4000000)
			{
				ptr = mmap_device_memory(0, ba[i].size,
						PROT_READ | PROT_WRITE | PROT_NOCACHE, 0, ba[i].addr);
				if (ptr == MAP_FAILED)
				{
					printf("Failed to map device memory thanks to error\n");
					exit(EXIT_FAILURE);
				}
				else
				{
					ptr_to_set = ptr;
//					ptr += 0x1000000;
//					for (int i = 0; i < UART_CHANNEL_COUNT; i++)
//					{
//					/* выведены	 p_uart[12] - p_uart[18]
//					 * p_uart[13] - p_uart[17]
//					 */
//						p_uart[i] = ptr;
//						channel[i].ptr_u = ptr;
//						ptr += 0x200;
//					}
					channel[0].ptr_u = p_uart[0] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x2000 + 0x600);
					channel[1].ptr_u = p_uart[1] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x2000 + 0x400);
					channel[2].ptr_u = p_uart[2] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x2000 + 0x200);
					channel[3].ptr_u = p_uart[3] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x2000 + 0x000);

					channel[4].ptr_u = p_uart[4] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x1800 + 0x600);
					channel[5].ptr_u = p_uart[5] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x1800 + 0x400);
					channel[6].ptr_u = p_uart[6] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x1800 + 0x200);
					channel[7].ptr_u = p_uart[7] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x1800 + 0x000);

					channel[8].ptr_u = p_uart[8] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x1000 + 0x600);
					channel[9].ptr_u = p_uart[9] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x1000 + 0x400);
					channel[10].ptr_u = p_uart[10] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x1000 + 0x200);
					channel[11].ptr_u = p_uart[11] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x1000 + 0x000);

					channel[12].ptr_u = p_uart[12] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x0800 + 0x600);
					channel[13].ptr_u = p_uart[13] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x0800 + 0x400);
					channel[14].ptr_u = p_uart[14] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x0800 + 0x200);
					channel[15].ptr_u = p_uart[15] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x0800 + 0x000);

					channel[16].ptr_u = p_uart[16] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x0000 + 0x600);
					channel[17].ptr_u = p_uart[17] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x0000 + 0x400);
					channel[18].ptr_u = p_uart[18] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x0000 + 0x200);
					channel[19].ptr_u = p_uart[19] = (p_uart_reg *)(ptr_to_set + 0x1000000 + 0x0000 + 0x000);
					/*-------------------------------------------------------------------------------------*/
					channel[20].ptr_u = p_uart[20] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x1000 + 0x600);
					channel[21].ptr_u = p_uart[21] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x1000 + 0x400);
					channel[22].ptr_u = p_uart[22] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x1000 + 0x200);
					channel[23].ptr_u = p_uart[23] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x1000 + 0x000);

					channel[24].ptr_u = p_uart[24] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x0800 + 0x600);
					channel[25].ptr_u = p_uart[25] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x0800 + 0x400);
					channel[26].ptr_u = p_uart[26] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x0800 + 0x200);
					channel[27].ptr_u = p_uart[27] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x0800 + 0x000);

					channel[28].ptr_u = p_uart[28] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x0000 + 0x600);
					channel[29].ptr_u = p_uart[29] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x0000 + 0x400);
					channel[30].ptr_u = p_uart[30] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x0000 + 0x200);
					channel[31].ptr_u = p_uart[31] = (p_uart_reg *)(ptr_to_set + 0x0000000 + 0x0000 + 0x000);


					port_init(ptr_to_set);
				}
			}
		}
	}

	int err = pci_device_read_irq(handler, &nirq, &irq[0]);

	if ((err == PCI_ERR_OK) && (nirq > 0))
	{
		uint_t i;
		for (i = 0; i < nirq; i++)
		{
			if (irq[i] != -1)
			{
				if ((id = InterruptAttach(irq[i], isr_handler, (void*) &dev_l,
						sizeof(dev_l), _NTO_INTR_FLAGS_TRK_MSK)) == -1)
				{
					perror("Can`t interrupt attach\n");
					exit(EXIT_FAILURE);
				}
				printf("Interrupt list: %u\n", irq[i]);
			}
		}
	}
	return id;
}
