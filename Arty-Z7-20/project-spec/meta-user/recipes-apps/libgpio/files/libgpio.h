/************************************************************************
*	This is the header file for a GPIO driver which is
*	implemented in an FPGA.
*
*	The GPIO hardware is the IP given by XILINX (AXI GPIO)
*	The hardware is controlled by a collection of registers 
*	implemented in the FPGA.
*	
*	To use the GPIO hardware, writes and reads to these registers must
*	take place at the correct times
*
*	The locations of these registers in memory are offsets of the GPIO base
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

#ifndef LIBGPIO_H
#define LIBGPIO_H

#include <stdint.h>
typedef uint8_t byte;
typedef void * GPIO;

/**********************HARDWARE REGISTER OFFSETS************************/
#define CHANNEL_1_DATA		0x00
#define CHANNEL_1_DIRECTION	0x04
#define CHANNEL_2_DATA		0x08
#define CHANNEL_2_DIRECTION	0x0C

/************************OPERATIONAL MASKS****************************/
#define INPUT				1
#define OUTPUT				0
#define HIGH				1
#define LOW					0

#define CHANNEL_MAX_SIZE		32
#define CHANNEL_MIN_SIZE		1

/**************************HELPFUL MACROS****************************/



/**************************FUNC DEFITIONS****************************/
GPIO GPIO_init(uint8_t uioNum, uint8_t mapNum);
int8_t setPinMode(GPIO vm, uint8_t channel, int8_t pinNum, int8_t direction);
int8_t digitalWrite(GPIO vm, uint8_t channel, int8_t pinNum, int8_t value);
int8_t digitalRead(GPIO vm, uint8_t channel, int8_t pinNum);
int8_t setChannelValue(GPIO vm, uint32_t valMask, int8_t channel);
uint32_t readChannelValue(GPIO vm, int8_t channel);
int8_t setChannelDirection(GPIO vm, uint32_t dirMask, int8_t channel);
uint8_t GPIO_Close(GPIO vm);
#endif