
#define RXFIFO_ENABLEPos	(2)
#define RXFIFO_ENABLE		(1<<RXFIFO_ENABLEPos)
#define RXFIFO_STATE(x)		(((x)&RXFIFO_ENABLE)>>RXFIFO_ENABLEPos)

#define TXFIFO_ENABLEPos	(1)
#define TXFIFO_ENABLE		(1<<TXFIFO_ENABLEPos)
#define TXFIFO_STATE(x)		(((x)&TXFIFO_ENABLE)>>TXFIFO_ENABLEPos)

#define IIR_IP_Pos			(0)
#define IIR_IP_Mask			((0x01)<<IIR_IP_Pos)
#define IIR_IP_Get(x)		(((x)&IIR_IP_Mask)>>IIR_IP_Pos)



#define IIR_Pos				(1)
#define IIR_Mask			((0x07)<<IIR_Pos)
#define IIR_Get(x)			(((x)&IIR_Mask)>>IIR_Pos)
#define IIR_Set				((0x07)<<IIR_Pos)

#define CMSR				(0x00)
#define RXFIFO_NOT_EMPTY	(0x02)
#define TXFIFO_EMPTY		(0x01)
#define ERROR_LINE			(0x03)
//#define ERROR_TIMEUOT		(0x06)

#define FE_Pos				(6)
#define FE_Mask				((0x03)<<FE_Pos)
#define FE_Get(x)			(((x)&FE_Mask)>>FE_Pos)
#define FE_Set(x)			(((x)<<FE_Pos)&FE_Mask)

#define MODE_RHR			(0x00)
#define MODE_THR			(0x01)
#define MODE_UART			(0x02)
#define MODE_UART_A			(0x03)

#define TXR_Pos				(0)
#define TXR_Mask			((0xFF)<<TXR_Pos)
#define TXR_Set(x)			(((x)<<TXR_Pos)&TXR_Mask)

#define RBR_Pos				(0)
#define RBR_Mask			((0xFF)<<RBR_Pos)
#define RBR_Get(x)			(((x)&RBR_Mask)>>RBR_Pos)

#define DLL_Pos				(0)
#define DLL_Mask			((0xFF)<<DLL_Pos)
#define DLL_Get(x)			(((x)&DLL_Mask)>>DLL_Pos)
#define DLL_Set(x)			(((x)<<DLL_Pos)&DLL_Mask)

#define DLH_Pos				(0)
#define DLH_Mask			((0xFF)<<DLH_Pos)
#define DLH_Get(x)			(((x)&DLH_Mask)>>DLH_Pos)
#define DLH_Set(x)			(((x)<<DLH_Pos)&DLH_Mask)

#define IER_CLEAR			(0x00)

#define IER_RxD_Pos			(0)
#define IER_RxD_Mask		((1)<<IER_RxD_Pos)
#define IER_RxD_Get(x)		(((x)&IER_RxD_Mask)>>IER_RxD_Pos)
#define IER_RxD_Set			(((1)<<IER_RxD_Pos)&IER_RxD_Mask)

#define IER_TxD_Pos			(1)
#define IER_TxD_Mask		((1)<<IER_TxD_Pos)
#define IER_TxD_Get(x)		(((x)&IER_TxD_Mask)>>IER_TxD_Pos)
#define IER_TxD_Set			(((1)<<IER_TxD_Pos)&IER_TxD_Mask)

#define IER_RxL_Pos			(2)
#define IER_RxL_Mask		((1)<<IER_RxL_Pos)
#define IER_RxL_Get(x)		(((x)&IER_RxL_Mask)>>IER_RxL_Pos)
#define IER_RxL_Set			(((1)<<IER_RxL_Pos)&IER_RxL_Mask)

#define IER_Mod_Pos			(3)
#define IER_Mod_Mask		((1)<<IER_Mod_Pos)
#define IER_Mod_Get(x)		(((x)&IER_Mod_Mask)>>IER_Mod_Pos)
#define IER_Mod_Set			(((1)<<IER_Mod_Pos)&IER_Mod_Mask)


#define FCR_FIFO_ENABLE_Pos		(0)
#define FCR_FIFO_ENABLE_Mask	((0x01)<<FCR_FIFO_ENABLE_Pos)
#define FCR_FIFO_ENABLE_Get(x)	(((x)&FCR_FIFO_ENABLE_Mask)>>FCR_FIFO_ENABLE_Pos)
#define FCR_FIFO_ENABLE_Set		(((1)<<FCR_FIFO_ENABLE_Pos)&FCR_FIFO_ENABLE_Mask)


#define FCR_RESETRF_Pos			(1)
#define FCR_RESETRF_Mask		((0x01)<<FCR_RESETRF_Pos)
#define FCR_RESETRF_Get(x)		(((x)&FCR_RESETRF_Mask)>>FCR_RESETRF_Pos)
#define FCR_RESETRF_Set			(((1)<<FCR_RESETRF_Pos)&FCR_RESETRF_Mask)

#define FCR_RESETTF_Pos			(2)
#define FCR_RESETTF_Mask		((0x01)<<FCR_RESETTF_Pos)
#define FCR_RESETTF_Get(x)		(((x)&FCR_RESETTF_Mask)>>FCR_RESETTF_Pos)
#define FCR_RESETTF_Set			(((1)<<FCR_RESETTF_Pos)&FCR_RESETTF_Mask)

#define FCR_ITLID_Pos			(6)
#define FCR_ITLID_Mask			((0x03)<<FCR_ITLID_Pos)
#define FCR_ITLID_Get(x)		(((x)&FCR_ITLID_Mask)>>FCR_ITLID_Pos)
#define FCR_ITLID_Set(x)		(((x)<<FCR_ITLID_Pos)&FCR_ITLID_Mask)

#define	ITL_MODE_01				(0x00)
#define	ITL_MODE_04				(0x01)
#define	ITL_MODE_08				(0x02)
#define	ITL_MODE_14				(0x03)

#define LCR_SDB_Pos				(0)
#define LCR_SDB_Mask			((0x03)<<LCR_SDB_Pos)
#define LCR_SDB_Get(x)			(((x)&LCR_SDB_Mask)>>LCR_SDB_Pos)
#define LCR_SDB_Set(x)			(((x)<<LCR_SDB_Pos)&LCR_SDB_Mask)

#define	LCR_SDB_MODE_05			(0x00)
#define	LCR_SDB_MODE_06			(0x01)
#define	LCR_SDB_MODE_07			(0x02)
#define	LCR_SDB_MODE_08			(0x03)

#define LCR_STOP_B_Pos			(2)
#define LCR_STOP_B_Mask			((0x01)<<LCR_STOP_B_Pos)
#define LCR_STOP_B_Get(x)		(((x)&LCR_STOP_B_Mask)>>LCR_STOP_B_Pos)
#define LCR_STOP_B_Set			(((1)<<LCR_STOP_B_Pos)&LCR_STOP_B_Mask)

#define LCR_PARENT_Pos			(3)
#define LCR_PARENT_Mask			((0x01)<<LCR_PARENT_Pos)
#define LCR_PARENT_Get(x)		(((x)&LCR_PARENT_Mask)>>LCR_PARENT_Pos)
#define LCR_PARENT_Set(x)		(((x)<<LCR_PARENT_Pos)&LCR_PARENT_Mask)

#define LCR_EVENPAR_Pos			(4)
#define LCR_EVENPAR_Mask		((0x01)<<LCR_EVENPAR_Pos)
#define LCR_EVENPAR_Get(x)		(((x)&LCR_EVENPAR_Mask)>>LCR_EVENPAR_Pos)
#define LCR_EVENPAR_Set(x)		(((x)<<LCR_EVENPAR_Pos)&LCR_EVENPAR_Mask)

#define LCR_STICPAR_Pos			(5)
#define LCR_STICPAR_Mask		((0x01)<<LCR_STICPAR_Pos)
#define LCR_STICPAR_Get(x)		(((x)&LCR_STICPAR_Mask)>>LCR_STICPAR_Pos)
#define LCR_STICPAR_Set(x)		(((x)<<LCR_STICPAR_Pos)&LCR_STICPAR_Mask)

#define LCR_BRCON_Pos			(6)
#define LCR_BRCON_Mask			((0x01)<<LCR_BRCON_Pos)
#define LCR_BRCON_Get(x)		(((x)&LCR_BRCON_Mask)>>LCR_BRCON_Pos)
#define LCR_BRCON_Set(x)		(((x)<<LCR_BRCON_Pos)&LCR_BRCON_Mask)



#define LCR_DLAB_Pos			(7)
#define LCR_DLAB_Mask			((0x01)<<LCR_DLAB_Pos)
#define LCR_DLAB_Get(x)			(((x)&LCR_DLAB_Mask)>>LCR_DLAB_Pos)
#define LCR_DLAB_Set			(((1)<<LCR_DLAB_Pos)&LCR_DLAB_Mask)



#define LSR_DR_Pos				(0)
#define LSR_DR_Mask				((0x01)<<LSR_DR_Pos)
#define LSR_DR_Get(x)			(((x)&LSR_DR_Mask)>>LSR_DR_Pos)
#define LSR_DR_Set(x)			(((x)<<LSR_DR_Pos)&LSR_DR_Mask)

#define LSR_OE_Pos				(1)
#define LSR_OE_Mask				((0x01)<<LSR_OE_Pos)
#define LSR_OE_Get(x)			(((x)&LSR_OE_Mask)>>LSR_OE_Pos)
#define LSR_OE_Set(x)			(((x)<<LSR_OE_Pos)&LSR_OE_Mask)

#define LSR_PE_Pos				(2)
#define LSR_PE_Mask				((0x01)<<LSR_PE_Pos)
#define LSR_PE_Get(x)			(((x)&LSR_PE_Mask)>>LSR_PE_Pos)
#define LSR_PE_Set(x)			(((x)<<LSR_PE_Pos)&LSR_PE_Mask)

#define LSR_FE_Pos				(3)
#define LSR_FE_Mask				((0x01)<<LSR_FE_Pos)
#define LSR_FE_Get(x)			(((x)&LSR_FE_Mask)>>LSR_FE_Pos)
#define LSR_FE_Set(x)			(((x)<<LSR_FE_Pos)&LSR_FE_Mask)

#define LSR_BD_Pos				(4)
#define LSR_BD_Mask				((0x01)<<LSR_BD_Pos)
#define LSR_BD_Get(x)			(((x)&LSR_BD_Mask)>>LSR_BD_Pos)
#define LSR_BD_Set(x)			(((x)<<LSR_BD_Pos)&LSR_BD_Mask)

#define LSR_THRE_Pos			(5)
#define LSR_THRE_Mask			((0x01)<<LSR_THRE_Pos)
#define LSR_THRE_Get(x)			(((x)&LSR_THRE_Mask)>>LSR_THRE_Pos)
#define LSR_THRE_Set			(((1)<<LSR_THRE_Pos)&LSR_THRE_Mask)

#define LSR_TEMPT_Pos			(6)
#define LSR_TEMPT_Mask			((0x01)<<LSR_TEMPT_Pos)
#define LSR_TEMPT_Get(x)		(((x)&LSR_TEMPT_Mask)>>LSR_TEMPT_Pos)
#define LSR_TEMPT_Set			(((1)<<LSR_TEMPT_Pos)&LSR_TEMPT_Mask)

#define LSR_FIFOE_Pos			(7)
#define LSR_FIFOE_Mask			((0x01)<<LSR_FIFOE_Pos)
#define LSR_FIFOE_Get(x)		(((x)&LSR_FIFOE_Mask)>>LSR_FIFOE_Pos)
#define LSR_FIFOE_Set(x)		(((x)<<LSR_FIFOE_Pos)&LSR_FIFOE_Mask)


/*********************************************************************/
#define SetNbit(x, n)		(x |= (1 << n))
#define ClearBit(x, n)		(x &= ~(1 << n))
#define TogglingBit(x, n)	(x ^= (1 << n))  // XOR
#define CheckBit(x, n) 		((x >> n) & 1)

#define MODE_08_TO_CS(x) 	(x * (0x10))
#define MODE_CS_TO_(x) 		(x / (0x10))
