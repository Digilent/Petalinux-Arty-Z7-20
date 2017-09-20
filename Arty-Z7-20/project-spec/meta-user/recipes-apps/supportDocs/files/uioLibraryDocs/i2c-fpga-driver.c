/************************************************************************
*	Author: Mitchell Orsucci
*	
*	This software is offered freely by Digilent Inc.
*
*	Creation Date: July 19, 2017
*	
*	To be used in tandem with a UIO driver and the Xilinx AXI IIC IP
*	that has been implemented on an FPGA
*	
*	Use at your own risk and peril
*
************************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include "uio-user.h"
#include "i2c-fpga.h"


/*************************STATIC FUNCTION DECLARATION*******************/
static void I2C_Write(I2C vm, byte slaveADX, byte * tx_buffer, byte numBytes, uint8_t option);

/*****************************************************************************/
/*!
	Initialize the I2C hardware to use in Dynamic mode


 @param	uioNum is the uio device number -- zero-indexed
 @param	mapNum is the specific map partition inside the UIO device
			if the UIO device contains only one partition, then the 
			map number is 0

 @return	an I2C (which is really a void * in disguise)
		This points to the location in virtual memory that the I2C Hardware
		has been mapped to

 @note		This library only allows for the use of five different AXI IIC
 		components. This limitation was picked to assist with software organization
 		and because most likely no one will need more than three I2C busses.

******************************************************************************/
I2C I2C_init(uint8_t uioNum, uint8_t mapNum) {
	if(uioNum < 0 || mapNum < 0) {
		fprintf(stderr, "That is not a valid UIO device or map number\n");
		fprintf(stderr, "Check /sys/class/uio for more information about"); 
		fprintf(stderr, " the available UIO devices\n");
		exit(EXIT_FAILURE);
	}
	void * vm;
	UIO * uio = UIO_MAP(uioNum, mapNum);
	vm = uio->mapPtr;
	ACCESS_REG(vm, RESETR) = SW_RST; /* Reset the hardware */
	ACCESS_REG(vm, RFD_REG) = (RX_FIFO_DEPTH - 1);	/* Set the FIFO depth */
	ACCESS_REG(vm, DGIER) = 0x00000000; /* Disable global interrupts */
	ACCESS_REG(vm, CR_REG) = (CR_TX_FIFO_RST);
	ACCESS_REG(vm, CR_REG) = (CR_ENABLE_DEVICE);
	uint32_t status = ACCESS_REG(vm, SR_REG);
	if(status != (SR_RX_EMPTY | SR_TX_EMPTY)) {
		fprintf(stderr, "FAILED TO CONFIGURE DEVICE PROPERLY\n");
	}
	return vm;
}



/*****************************************************************************/
/*!
	Write some data onto the I2C bus directed at the I2C address specified by
	slaveADX, with a stop condition on the bus at the end of transfer
	i.e. give up control of the I2C bus at the end of the transfer


	@param	vm is a pointer to the location in virtual memory where the I2C
			hardware has been mapped to
 	@param	slaveADX is the 7-bit slave address of the device to
			communicate with
 	@param	tx_buffer is a pointer to the location where the data to transfer resides
 	@param	numBytes is the number of bytes to transfer

******************************************************************************/
void I2C_WriteWithStop(I2C vm, byte slaveADX, byte * tx_buffer, byte numBytes) {
	I2C_Write(vm, slaveADX, tx_buffer, numBytes, WITH_STOP);
}



/*****************************************************************************/
/*!
	Write some data onto the I2C bus directed at the I2C address specified by
	slaveADX, without a stop condition on the bus at the end of transfer
	This can be useful when some information needs to be written to a device
	before subsequently reading from the device
	i.e. do not give up control of the I2C bus at the end of the transfer


 	@param	vm is a pointer to the location in virtual memory where the I2C
			hardware has been mapped to
 	@param	slaveADX is the 7-bit slave address of the device to
			communicate with
 	@param	tx_buffer is a pointer to the location where the data to transfer resides
 	@param	numBytes is the number of bytes to transfer

******************************************************************************/
void I2C_WriteWithoutStop(I2C vm, byte slaveADX, byte * tx_buffer, byte numBytes) {
	I2C_Write(vm, slaveADX, tx_buffer, numBytes, WITHOUT_STOP);
}


/************************************************************************************
	Static function that streamlines the different processes of writing
	with or without a stop
*************************************************************************************/
static void I2C_Write(I2C vm, byte slaveADX, byte * tx_buffer, byte numBytes, uint8_t option) {

	while(ACCESS_REG(vm, SR_REG) & SR_BUSBUSY) {
		/* Wait for the bus to become free */
	}

	/* Reset the int_tx bit */
	ACCESS_REG(vm, IISR) = ACCESS_REG(vm, IISR) & (INT_TX_EMPTY | INT_ARB_LOST | INT_TX_ERROR);

	/* Format the slave address and place it into TX FIFO */
	uint16_t adxMask = 0;
	adxMask = (slaveADX << 1);
	adxMask = (DYN_START) | (adxMask | WRITE_OP);
	ACCESS_REG(vm, DTR_REG) = adxMask;
	
	uint32_t status = ACCESS_REG(vm, SR_REG);
	while((status & SR_BUSBUSY) != SR_BUSBUSY) {
		status = ACCESS_REG(vm, SR_REG);
		/* Wait for the bus to become busy */
	}

	/* Clear the BNB interrupt, which must happen while the bus is busy */
	ACCESS_REG(vm, IISR) = ACCESS_REG(vm, IISR) & INT_BNB;

	uint32_t intStatus;
	while(numBytes > 0) {
		while(1) {
			intStatus = ACCESS_REG(vm, IISR);
			/* Check for transfer errors */
			if(intStatus & (INT_TX_ERROR | INT_BNB | INT_ARB_LOST)) {
				ACCESS_REG(vm, CR_REG) = (CR_ENABLE_DEVICE | CR_TX_FIFO_RST);
				ACCESS_REG(vm, CR_REG) = (CR_ENABLE_DEVICE);
				printf("An I2C write error was detected....\n");
				switch(intStatus & (INT_TX_ERROR | INT_BNB | INT_ARB_LOST)) {
					case INT_TX_ERROR:
						printf("TX_ERROR\n");
					case INT_BNB:
						printf("BNB ERROR\n");
					case INT_ARB_LOST:
						printf("ARBITRATION LOST\n");
				}
				return;
			}
			
			/* Wait for TX_FIFO to become empty before sending any more daya */
			if(intStatus & INT_TX_EMPTY) {
				break;
			}

		}

		/* Specify to only send stop condition under specific circumstances */
		if((numBytes == 1) && (option == WITH_STOP)) {
			ACCESS_REG(vm, DTR_REG) = (DYN_STOP | *tx_buffer++);
		} else {
			ACCESS_REG(vm, DTR_REG) = *tx_buffer++;
		}
		numBytes--;
	}

	if(option == WITH_STOP) {
		while(1) {
			/* Wait for the bus to transition to not busy */
			if(ACCESS_REG(vm, SR_REG) & SR_BUSBUSY) {
				break;
			}
		}
	}

}


/*****************************************************************************/
/*!
	Requests a certain of number of bytes to be transferred in from the device
	located at the specified slaveADX


 	@param	vm is a pointer to the location in virtual memory where the I2C
			hardware has been mapped to
 	@param	slaveADX is the 7-bit slave address of the device to
			communicate with
 	@param	rx_buffer is a pointer to the location where the data to be received
			will be placed throughout the transfer.
 	@param	numBytes is the number of bytes to read from the slave device

******************************************************************************/
void I2C_Read(I2C vm, byte slaveADX, byte * rx_buffer, byte numBytes) {
	/* Clear the TXempty, TXerror, and ARBlost interrupts */
	ACCESS_REG(vm, IISR) = ACCESS_REG(vm, IISR) & (INT_TX_EMPTY | INT_ARB_LOST | INT_TX_ERROR);

	/* Format the address and place it into the TX FIFO */
	uint16_t adxMask = 0;
	adxMask = (slaveADX << 1);
	adxMask |= (READ_OP | DYN_START);
	ACCESS_REG(vm, DTR_REG) = adxMask;

	while(!(ACCESS_REG(vm, SR_REG) & SR_BUSBUSY)) {
		/* Wait for the bus to transition to a busy state */
	}

	/* Clear the bus-not-busy interrupt bit */
	ACCESS_REG(vm, IISR) = ACCESS_REG(vm, IISR) & INT_BNB;

	/* Format the number of bytes to read in before sending a NACK */
	uint16_t byteCount = (numBytes | DYN_STOP);

	/* place the stop condition and number of bytes into TX FIFO */
	ACCESS_REG(vm, DTR_REG) = byteCount;

	uint32_t intMask;
	while(numBytes > 0) {
		/* Set flag condition based on number of bytes left to receive */
		if(numBytes == 1) {
			intMask = (INT_ARB_LOST | INT_BNB);
		} else {
			intMask = (INT_ARB_LOST | INT_TX_ERROR | INT_BNB);
		}

		while(1) {
			if(!(ACCESS_REG(vm, SR_REG) & SR_RX_EMPTY)) {
				/* Wait til there is something in the RX FIFO */
				break;
			}

			if(ACCESS_REG(vm, IISR) & intMask) {
				fprintf(stderr, "An error has occurred on read\n\r");
				switch(ACCESS_REG(vm, IISR) & intMask) {
					case INT_ARB_LOST:
						fprintf(stderr, "ARBITRATION LOST\n");
					case INT_BNB:
						fprintf(stderr, "BUS NOT BUSY ERROR\n");
					case INT_TX_ERROR:
						fprintf(stderr, "TX ERROR\n");
				}

				return;
			}
		}
		/* Get the data from the fifo and place it in the buffer */
		*rx_buffer++ = ACCESS_REG(vm, DRR_REG);
		numBytes--;
	}
}

/*****************************************************************************/
/*!
	Unmaps the I2C hardware from virtual memory and frees all items associated
	with the UIO device


 	@param	vm is a pointer to the location in virtual memory where the I2C
			hardware has been mapped to

******************************************************************************/
uint8_t I2C_Close(I2C vm) {
	return UIO_UNMAP(vm);
}

