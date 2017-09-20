/************************************************************************
*	Author: Mitchell Orsucci
*	
*	This software is offered freely by Digilent Inc.
*
*	Creation Date: July 24, 2017
*	
*	To be used in tandem with a UIO driver and the Xilinx AXI QUAD SPI IP
*	that has been implemented on an FPGA
*	
*	Use at your own risk and peril
*
************************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include "uio-user.h"
#include "spi-fpga.h"

/*****************************************************************************/
/*!
	Initialize the SPI hardware to use in Master mode with 8-bit transfer width
	This SPI hardware should work with FIFO depth of 1, 16, or 256


 @param	uioNum is the uio device number -- zero-indexed
 @param	mapNum is the specific map partition inside the UIO device
			if the UIO device contains only one partition, then the 
			map number is 0

 @return	a SPI (which is really a void * in disguise)
		This points to the location in virtual memory that the SPI Hardware
		has been mapped to, and is passed to all other functions in this driver
		for control of the SPI hardware

 @note 	The SPI hardware is configured with SPI Mode 0. To set another SPI mode
 		for the hardware, use SPI_setMode() to set the specific mode for 
 		CPOL and CPHA

******************************************************************************/
SPI SPI_init(uint8_t uioNum, uint8_t mapNum) {
	if(uioNum < 0 || mapNum < 0) {
		printf("That is not a valid UIO device or map number\n");
		printf("Check /sys/class/uio for more information about"); 
		printf(" the available UIO devices\n");
		exit(EXIT_FAILURE);
	}
	void * vm;
	UIO * uio = UIO_MAP(uioNum, mapNum);
	vm = uio->mapPtr;
	ACCESS_REG(vm, SPI_SRR) = SPI_RST; // Reset the hardware
	usleep(1000);
	
	/* Configure the SPI to begin in mode CPOL = 0, CPHA = 0
		User can change the SPI mode by using SPI_setMode() */
	ACCESS_REG(vm, SPI_CR) = (CR_MAN_SLAVE_SEL | CR_RST_RX | CR_RST_TX | CR_MSTR_MODE); // Set controls for the device
	ACCESS_REG(vm, SPI_GLB_INT) = 0x00000000; // Disable global interrupt
	ACCESS_REG(vm, SPI_SS) = NO_SLAVES_SELECTED; /* Select Slave*/
	ACCESS_REG(vm, SPI_IIER) = INT_TX_EMPTY;
 	return vm;
}

/*****************************************************************************/
/*!
	Sets the mode of data transfer for the SPI hardware by specifying the 
	Polarity and phase of data transfers. See the specific documentation for your
	SPI device to see which mode it needs for proper functionality


 @param	SPI vm is SPI device to be worked on
 @param	CPOL is the mode to set the SPI device to
 		if 0: CPOL is set to mode zero
 		if !0: CPOL is set to mode one
 @param	CPHA is the mode to set the SPI device to
 		if 0: CPHA is set to mdoe zero
 		if !0: CPHA is set to mode one

 @note There are only 4 valid SPI modes:
 	CPOL = 0, CPHA = 0  	mode 0
 	CPOL = 1, CPHA = 0		mode 1
 	CPOL = 0, CPHA = 1		mode 2
 	CPOL = 1, CPHA = 1		mode 3
 	Modes 0 and 3 are the most common
******************************************************************************/
void SPI_setMode(SPI vm, byte CPOL, byte CPHA) {
	uint32_t config_reg = ACCESS_REG(vm, SPI_CR);

	/* Clears the current CPOL and CPHA settings */
	config_reg &= ~(CR_CPHA | CR_CPOL);

	/* Adds in the new settings and writes them to the config register */
	config_reg |= ((!!CPOL) << 3);
	config_reg |= ((!!CPHA) << 4);
	ACCESS_REG(vm, SPI_CR) = config_reg;
}

/*****************************************************************************/
/*!
	Transfers numBytes of data from the tx_buffer to the slave device
	and receives numBytes of data from the slave device into rx_buffer
	If rx_buffer is NULL, the received data from the slave is disregarded


 @param	SPI vm is SPI device to be worked on
 @param	tx_buffer is a pointer to the location where the data to be transferred
 		is stored. The tx_buffer must be of data size byte, char, int8 or uint8
 @param	rx_buffer is a pointer to the location where the data to be received will
 		be placed. Can be NULL if data to be received is arbitrary
 @param numBytes is the number of bytes to be transferred or received, but
 		realistically is limited by the FIFO depth specified in the AXI QUAD SPI
 		setup. If Fifo depth is 16, then numBytes can not be larger than 16

 @note rx_buffer can be NULL, but tx_buffer cannot be NULL. If one is looking to
 		receive data from a slave while needing to send arbitrary data, tx_buffer
 		can be padded with arbitrary data to send.
******************************************************************************/
void SPI_Transfer(SPI vm, byte * tx_buffer, byte * rx_buffer, byte numBytes) {
	if(rx_buffer == NULL) {
		byte dummy[numBytes];
		rx_buffer = dummy;
	}
	byte tx_byteCount = numBytes;
	byte rx_byteCount = numBytes;
	ACCESS_REG(vm, SPI_SS) = SLAVE_1;
	
	/* Fill the TX FIFO with numBytes from tx_buffer */
	while(tx_byteCount > 0) {
		ACCESS_REG(vm, SPI_DTX) = *tx_buffer++;
		tx_byteCount--;
	}

	/* Enable the hardware, so as to put the data on the bus */
	ACCESS_REG(vm, SPI_CR) |= CR_SPI_ENABLE;

	while(!(ACCESS_REG(vm, SPI_IISR) & INT_TX_EMPTY)) {
		/* Wait til the TX FIFO is empty */
		/* Then we know all data has been transferred */
	}

	ACCESS_REG(vm, SPI_SS) = NO_SLAVES_SELECTED; /* Select no slaves */
	ACCESS_REG(vm, SPI_CR) &= ~CR_SPI_ENABLE;	/* Disable the hardware from making further transfers */
	ACCESS_REG(vm, SPI_IISR) |= INT_TX_EMPTY;	/* Reset the TX Empty interrupt */

	while(rx_byteCount > 0) {
		*rx_buffer++ = ACCESS_REG(vm, SPI_DRX);
		rx_byteCount--;
	}

}

/*****************************************************************************/
/*!
	Closes and frees all memory associated with the SPI device


 @param	SPI vm is SPI device to be worked on

 @return returns 0 if Close was accomplished properly
 		returns -1 otherwise
******************************************************************************/
uint8_t SPI_Close(SPI vm) {
	return UIO_UNMAP(vm);
}