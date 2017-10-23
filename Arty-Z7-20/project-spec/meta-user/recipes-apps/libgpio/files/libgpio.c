/************************************************************************
*	Author: Mitchell Orsucci
*	
*	This software is offered freely by Digilent Inc.
*
*	Creation Date: July 24, 2017
*	
*	To be used in tandem with a UIO driver and the Xilinx AXI GPIO
*	that has been implemented on an FPGA
*	
*	Use at your own risk and peril
*
************************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include <libuio.h>
#include <libgpio.h>

/*****************************************************************************/
/*!
	Initialize the GPIO hardware. This function sets the default value of both
	channels of GPIO to output mode with a value of LOW


 @param	uioNum is the uio device number -- zero-indexed
 @param	mapNum is the specific map partition inside the UIO device
			if the UIO device contains only one partition, then the 
			map number is 0

 @return	a GPIO (which is really a void * in disguise)
		This points to the location in virtual memory that the GPIO Hardware
		has been mapped to, and is passed to all other functions in this driver
		for control of the GPIO hardware

 @note  This function sets both channels of GPIO to output mode, set LOW

******************************************************************************/
GPIO GPIO_init(uint8_t uioNum, uint8_t mapNum) {
	if(uioNum < 0 || mapNum < 0) {
		fprintf(stderr, "That is not a valid UIO device or map number\n");
		fprintf(stderr, "Check /sys/class/uio for more information about"); 
		fprintf(stderr, " the available UIO devices\n");
		exit(EXIT_FAILURE);
	}
	void * vm;
	UIO * uio = UIO_MAP(uioNum, mapNum);
	vm = uio->mapPtr;
	
 	return vm;
}

/*****************************************************************************/
/*!
	Sets the mode of a specific GPIO pin with value between 1 & 32
		valid modes are INPUT or OUTPUT

 @param	GPIO vm, the GPIO device to be worked on
 @param	pinNum is the number of the GPIO that should be modified
 		This is a one-indexed pin. SO the first pin in the GPIO array
 		is number 1, and the last pin is number 32
 @param direction is the mode to set the pinNum pin to
 		OUTPUT sets it to output mode
 		INPUT sets it to input mode

 @return 0 if successful
 		-1 otherwise

******************************************************************************/
int8_t setPinMode(GPIO vm, uint8_t channel, int8_t pinNum, int8_t direction) {
	if(pinNum > CHANNEL_MAX_SIZE || pinNum < CHANNEL_MIN_SIZE || direction > INPUT || direction < OUTPUT) {
		fprintf(stderr, "Invalid Argument passed to setPinMode()\n");
		fprintf(stderr, "Please look at the valid function arguments\n");
		return -1;
	}

	/* Get the current settings for direction */
	uint32_t currentDir = ACCESS_REG(vm, CHANNEL_1_DIRECTION + ((channel - 1) * 8));

	/* Clear the bit in question */
	currentDir &= ~(1 << (pinNum - 1));
	currentDir |= (direction << (pinNum - 1));

	/* Set the new configuration */
	ACCESS_REG(vm, CHANNEL_1_DIRECTION + ((channel - 1) * 8)) = currentDir;
	return 0;
}

/*****************************************************************************/
/*!
	Sets the value of a specific GPIO pin as referenced by pinNum

 @param	GPIO vm, the GPIO device to be worked on
 @param	pinNum is the number of the GPIO that should be modified
 		This is a one-indexed pin. SO the first pin in the GPIO array
 		is number 1, and the last pin is number 32
 @param value is the value to set the pinMode
 		HIGH and anything nonzero will set the pin to HIGH
 		LOW and 0 will set the pin to low

 @return 0 if successful
 		-1 otherwise

 @note the specified pin must be set to OUTPUT mode first
 		a write to a pin that is INPUT mode has no effect

******************************************************************************/
int8_t digitalWrite(GPIO vm, uint8_t channel, int8_t pinNum, int8_t value) {
	if(pinNum > CHANNEL_MAX_SIZE || pinNum < CHANNEL_MIN_SIZE || value < LOW) {
		fprintf(stderr, "Invalid Argument passed to digitalWrite()\n");
		fprintf(stderr, "Please look at the valid function arguments\n");
		return -1;
	}
	value = !!value; /* convert any thing over 0 to a 1 */

	/* Get the current settings for value */
	uint32_t currentVal = ACCESS_REG(vm, CHANNEL_1_DATA + ((channel - 1) * 8));
		/* Clear the bit in question */
	currentVal &= ~(1 << (pinNum - 1));
	currentVal |= (value << (pinNum - 1));
	
	ACCESS_REG(vm, CHANNEL_1_DATA + ((channel - 1) * 8)) = currentVal;
	return 0;
}

/*****************************************************************************/
/*!
	reads the value of a specific GPIO pin as referenced by pinNum

 @param	GPIO vm, the GPIO device to be worked on
 @param	pinNum is the number of the GPIO that should be modified
 		This is a one-indexed pin. SO the first pin in the GPIO array
 		is number 1, and the last pin is number 32
 @param value is the value to set the pinMode
 		HIGH and anything nonzero will set the pin to HIGH
 		LOW and 0 will set the pin to low

 @return 0 if the pin is LOW
 		 1 if the pin is HIGH
 		 -1 if an error has occurred

 @note the specified pin must be set to INPUT mode first
 		a read of a pin that is in OUTPUT mode is undefined behavior
 		and will return an arbitrary value

******************************************************************************/
int8_t digitalRead(GPIO vm, uint8_t channel, int8_t pinNum) {
	if(pinNum > CHANNEL_MAX_SIZE || pinNum < CHANNEL_MIN_SIZE) {
		printf("Invalid Argument passed to digitalRead()\n");
		printf("Please look at the valid function arguments\n");
		return -1;
	}

	return ((ACCESS_REG(vm, CHANNEL_1_DATA + ((channel - 1) * 8)) >> (pinNum - 1)) & 0x01);
}


/*****************************************************************************/
/*!
	Sets the value of an entire GPIO channel according to valMask

 @param	GPIO vm, the GPIO device to be worked on
 @param	valMask is the bitmask of values to be set on the GPIO channel
 @param channel is the GPIO channel to work on
 		only values of 1 and 2 are valid for channel

 @return 0 if successful
 		-1 otherwise

 @note the specified channel must be set to OUTPUT mode first
 		a write to a channel that is INPUT mode has no effect
		
		both channels must be enabled when AXi GPIO is implemented
		if channel 2 is to be written to.
		writing to a channel that is not enabeled is undefined

******************************************************************************/
int8_t setChannelValue(GPIO vm, uint32_t valMask, int8_t channel){
	if(channel > 2 || channel < 1) {
		fprintf(stderr, "Invalid Argument passed to setChannelValue()\n");
		fprintf(stderr, "Please look at the valid function arguments.\n");
		return -1;
	}

	ACCESS_REG(vm, CHANNEL_1_DATA + ((channel - 1) * 8)) = valMask;
	return 0;
}


/*****************************************************************************/
/*!
	Sets the value of an entire GPIO channel according to valMask

 @param	GPIO vm, the GPIO device to be worked on
 @param channel is the GPIO channel to work on
 		only values of 1 and 2 are valid for channel

 @return the value of the channel to be read as a uint32 if successful
 		-1 otherwise

 @note the specified channel must be set to INPUT mode first
 		a read from a channel that is in OUTPUT is undefined
		
		both channels must be enabled when AXi GPIO is implemented
		if channel 2 is to be written to.
		writing to a channel that is not enabeled is undefined

******************************************************************************/
uint32_t readChannelValue(GPIO vm, int8_t channel) {
	if(channel > 2 || channel < 1) {
		printf("Invalid Argument passed to readChannelValue()\n");
		printf("Please look at the valid function arguments.\n");
		return -1;
	}

	return ACCESS_REG(vm, CHANNEL_1_DATA + ((channel - 1) * 8));
}

/*****************************************************************************/
/*!
	Sets the mode of an entire GPIO channel according to dirMask

 @param	GPIO vm, the GPIO device to be worked on
 @param dirMask, a bitmask of values of modes for each specific pin
 		in an channel. (1 corresponds to INPUT, 0 corresponds to OUTPUT)
 @param channel is the GPIO channel to work on
 		only values of 1 and 2 are valid for channel

 @return 0 if successful
 		-1 otherwise

 @note both channels must be enabled when AXi GPIO is implemented
		if channel 2 is to be written to.
		writing to a channel that is not enabeled is undefined

******************************************************************************/
int8_t setChannelDirection(GPIO vm, uint32_t dirMask, int8_t channel) {
	if(channel > 2 || channel < 1) {
		printf("Invalid Argument passed to setChannelDirection()\n");
		printf("Please look at the valid function arguments.\n");
		return -1;
	}
	ACCESS_REG(vm, CHANNEL_1_DIRECTION + ((channel - 1) * 8)) = dirMask;
	return 0;
}

/*****************************************************************************/
/*!
	Closes and frees all memory associated with the GPIO device


 @param	GPIO vm is the GPIO device to be worked on

 @return returns 0 if Close was accomplished properly
 		returns -1 otherwise
******************************************************************************/
uint8_t GPIO_Close(GPIO vm) {
	return UIO_UNMAP(vm);
}