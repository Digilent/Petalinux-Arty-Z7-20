/************************************************************************
*	This is the header file for a SPI protocol driver which is
*	implemented in an FPGA.
*
*	The SPI hardware is the IP given by XILINX (AXI QUAD SPI)
*	The hardware is controlled by a collection of registers 
*	implemented in the FPGA.
*	
*	To use the SPI hardware, writes and reads to these registers must
*	take place at the correct times
*
*	The locations of these registers in memory are offsets of the SPI base
*	address.
*	
************************************************************************/ 

/************************************************************************
*	Author: Mitchell Orsucci
*	
*	This software is offered freely by Digilent Inc.
*
*	Creation Date: July 24, 2017
*	
*
************************************************************************/ 

#ifndef SPI_FPGA_H
#define SPI_FPGA_H

#include <stdint.h>
typedef uint8_t byte;
typedef void * SPI;

/**********************HARDWARE REGISTER OFFSETS************************/
/* Taken Straight from the specification document and provided Xilinx lib */
#define SPI_GLB_INT 	0x1c /* Global Intr Enable Reg */
#define SPI_IISR 		0x20 /* Interrupt Status Reg */
#define SPI_IIER 		0x28 /* Interrupt Enable Reg */
#define SPI_SRR 		0x40 /* Software Reset Register */
#define SPI_CR 			0x60 /* Control Register */
#define SPI_SR 			0x64 /* Status Register */
#define SPI_DTX 		0x68 /* Data Transmit Register */
#define SPI_DRX 		0x6c /* Data Receive Register */
#define SPI_SS 			0x70 /* Slave Select Register */
#define SPI_TFO 		0x74 /* Tx Fifo Occupancy */
#define SPI_RFO 		0x78 /* Rx Fifo Occupancy */

/*********************STATUS AND INTERRUPT MASKS**********************/
/* These are masks for reading the interrupt status register SPI_IISR */
#define INT_MODE_FLT		0x00000001 /**< Mode fault error */
#define INT_SLAVE_MODE_FLT	0x00000002 /**< Selected as slave while disabled */
#define INT_TX_EMPTY		0x00000004 /**< DTR/TxFIFO is empty */
#define INT_TX_UNDERRUN		0x00000008 /**< DTR/TxFIFO underrun */
#define INT_RX_FULL			0x00000010 /**< DRR/RxFIFO is full */
#define INT_RX_UNDERRUN		0x00000020 /**< DRR/RxFIFO overrun */
#define INT_TX_HALF_EMPTY	0x00000040 /**< TxFIFO is half empty */
#define INT_SLAVE_MODE		0x00000080 /**< Slave select mode */
#define INT_RX_NOT_EMPTY	0x00000100 /**< RxFIFO not empty */


/* These are masks for reading the status register SPI_SR */
#define SR_RX_EMPTY 	0x00000001 	/* RX FIFO is empty */
#define SR_RX_FULL 		0x00000002 	/* RX FIFO is full */
#define SR_TX_EMPTY		0x00000004 	/* TX FIFO is empty */
#define SR_TX_FULL		0x00000008 	/* TX FIFO is full */



/* These are masks for reading/writing the control register SPI_CR*/
#define CR_LSB_FIRST		0x00000200	/* Set mode to MSB first or LSB first */
#define CR_MSTR_INHIBIT		0x00000100	/* Master transactions inhibited when hi */
#define	CR_MAN_SLAVE_SEL	0x00000080	/* Determine if slave select follows the contents of SPI_SS */
#define CR_RST_RX			0x00000040 	/* Reset RX FIFO */
#define CR_RST_TX			0x00000020 	/* Reset TX FIFO */
#define CR_CPHA				0x00000010 	/* SPI mode CPHA control */
#define CR_CPOL				0x00000008 	/* SPI mode CPOL control */
#define CR_MSTR_MODE		0x00000004 	/* SPI Master mode enable */
#define CR_SPI_ENABLE		0x00000002 	/* Enables the SPI hardware */
#define CR_LOOPBACK 		0x00000001 	/* Enables Loopback mode */


/************************OPERATIONAL MASKS****************************/
#define NO_SLAVES_SELECTED	-1			/* No slaves are active */
#define SLAVE_1				-2			/* Slave one is active */
#define SLAVE_2 			0xFFFFFFFD 	/* Slave two is active */

#define SPI_RST				0x0000000a	/* Software Reset 		*/


/**************************HELPFUL MACROS****************************/


/**************************FUNC DEFITIONS****************************/
SPI SPI_init(uint8_t uioNum, uint8_t mapNum);
void SPI_setMode(SPI vm, byte CPOL, byte CPHA);
void SPI_Transfer(SPI vm, byte * tx_buffer, byte * rx_buffer, byte numBytes);
uint8_t SPI_Close(SPI vm);
#endif