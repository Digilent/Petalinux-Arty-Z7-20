/************************************************************************
*	This is the header file for a PWM driver which is
*	implemented in an FPGA.
*
*	The GPIO hardware is the IP given by Digilent (PWM v2.0)
*	The hardware is controlled by a collection of registers 
*	implemented in the FPGA.
*	
*	To use the PWM hardware, writes and reads to these registers must
*	take place at the correct times and in the correct fashion
*   This library abstracts these register accesses away
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
*	Creation Date: August 11, 2017
*	
*
************************************************************************/ 
#ifndef LIBPWM_H
#define LIBPWM_H

#include <stdint.h>
typedef uint8_t byte;
typedef void * PWM;

/**********************HARDWARE REGISTER OFFSETS************************/
#define PWM_CTRL_OFFSET         0
#define PWM_PERIOD_OFFSET       0x8
#define PWM_DUTY_OFFSET         0x40

/**************************FUNC DEFITIONS****************************/
PWM PWM_init(uint8_t uioNum, uint8_t mapNum);
void PWM_Enable(PWM pwm);
void PWM_Disable(PWM pwm);
void setPwmDuty(PWM pwm, uint8_t pwmNum, uint32_t duty);
void setPwmPeriod(PWM pwm, uint32_t period);
uint32_t getPwmDuty(PWM pwm, uint8_t pwmNum);
uint32_t getPwmPeriod(PWM pwm, uint8_t pwmNum);
uint8_t PWM_Close(PWM pwm);
#endif