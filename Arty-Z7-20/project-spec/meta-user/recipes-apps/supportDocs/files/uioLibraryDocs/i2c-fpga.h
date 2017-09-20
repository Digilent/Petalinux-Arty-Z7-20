/************************************************************************
*	This is the header file for an i2c protocol driver which is
*	implemented in an FPGA.
*
*	The i2c hardware is the IP given by XILINX (AXI IIC)
*	The hardware is controlled by a collection of registers 
*	implemented in the FPGA.
*	
*	To use the i2c hardware, writes and reads to these registers must
*	take place at the correct times
*
*	The locations of these registers in memory are offsets of the i2c base
*	address.
*	
************************************************************************/ 

/************************************************************************
*	Author: Mitchell Orsucci
*	
*	This software is offered freely by Digilent Inc.
*
*	Creation Date: July 19, 2017
*	
*
************************************************************************/ 

#ifndef I2C_FPGA_H
#define I2C_FPGA_H

#include <stdint.h>
typedef uint8_t byte;
typedef void * I2C;


/**********************HARDWARE REGISTER OFFSETS************************/
/* Taken Straight from XIIC specification document and provided Xilinx lib */
#define DGIER	0x1C  	/**< Global Interrupt Enable Register */
#define IISR	0x20  	/**< Interrupt Status Register */
#define IIER	0x28  	/**< Interrupt Enable Register */
#define RESETR	0x40  	/**< Reset Register */
#define CR_REG	0x100 	/**< Control Register */
#define SR_REG	0x104 	/**< Status Register */
#define DTR_REG	0x108 	/**< Data Tx Register */
#define DRR_REG	0x10C 	/**< Data Rx Register */
#define ADR_REG	0x110 	/**< Slave Address Register */
#define TFO_REG	0x114 	/**< Tx FIFO Occupancy */
#define RFO_REG	0x118 	/**< Rx FIFO Occupancy */
#define TBA_REG	0x11C 	/**< 10 Bit Address reg */
#define RFD_REG	0x120 	/**< Rx FIFO Depth reg */
#define GPO_REG	0x124 	/**< General Purpose Output Register */
#define TSUSTA	0x128
#define TSUSTO	0x12C
#define THDSTA	0x130
#define TSUDAT 	0x134
#define TBUF 	0x138
#define THIGH	0x13C
#define TLOW	0x140
#define THDDAT	0x144


/*********************STATUS AND INTERRUPT MASKS**********************/
/* These are masks for reading the interrupt status register IISR */
#define GLBL_INTRPT_ENABLE	0x80000000  /* Global interrupt enable */
#define INT_ARB_LOST		0x00000001	/* Arbitration lost 	*/
#define INT_TX_ERROR		0x00000002	/* Transmit error 		*/
#define INT_TX_EMPTY		0x00000004	/* Transmit FIFO empty 	*/
#define INT_RX_FULL			0x00000008	/* Receive FIFO full 	*/
#define INT_BNB				0x00000010 	/* Bus not busy			*/
#define INT_AAS				0x00000020 	/* Addressed as Slave 	*/
#define INT_NAAS			0x00000040 	/* Not addressed as Slave */
#define INT_TX_HALF			0x00000080  /* TX FIFO half empty 	*/

/* These are masks for reading the status register SR_REG		*/
#define SR_GEN_CALL			0x00000001 	/* A master issued a general call */
#define SR_AAS				0x00000002 	/* Addressed as slave */
#define SR_BUSBUSY			0x00000004  /* Bus is busy */
#define SR_MSTR_RDING_SLAVE	0x00000008 	/* Master reading from slave */
#define SR_TX_FULL			0x00000010 	/* TX FIFO full 	*/
#define SR_RX_FULL			0x00000020 	/* RX FIFO full 	*/
#define SR_RX_EMPTY			0x00000040  /* RX FIFO empty 	*/
#define SR_TX_EMPTY			0x00000080 	/* TX FIFO empty 	*/

/* These are masks for setting/reading the config register CR_REG	*/
#define CR_ENABLE_DEVICE	0x00000001 	/* Enables the device */
#define CR_TX_FIFO_RST		0x00000002 	/* Resets the TX FIFO */

/************************OPERATIONAL MASKS****************************/
#define SW_RST				0x0000000a	/* Software Reset 		*/

#define DYN_START			0x00000100 	/* Dynamic Start mask 	*/
#define DYN_STOP			0x00000200 	/* Dynamic Stop mask 	*/
#define TX_FIFO_DEPTH		16
#define RX_FIFO_DEPTH		16

#define READ_OP				0x00000001 	/* For Read Operations */
#define WRITE_OP			0x00000000  /* For Write Operations */

#define WITH_STOP			0x00000001  /* To send a stop bit at end of write */
#define WITHOUT_STOP		0x00000000	/* To send without a stop bit at end of write */

/**************************HELPFUL MACROS****************************/


/**************************FUNC DEFITIONS****************************/
I2C I2C_init(uint8_t uioNum, uint8_t mapNum);
void I2C_WriteWithStop(I2C vm, byte slaveADX, byte * tx_buffer, byte numBytes);
void I2C_WriteWithoutStop(I2C vm, byte slaveADX, byte * tx_buffer, byte numBytes);
void I2C_Read(I2C vm, byte slaveADX, byte * rx_buffer, byte numBytes);
uint8_t I2C_Close(I2C vm);

#endif