/************************************************************************
*	Author: Mitchell Orsucci
*	
*	This software is offered freely by Digilent Inc.
*
*	Creation Date: August 11, 2017
*	
*	To be used in tandem with a UIO driver and the Digilent PWM v2.0 IP core
*	that has been implemented on an FPGA
*	
*	Use at your own risk and peril
*
************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <libuio.h>
#include <libpwm.h>
 
PWM PWM_init(uint8_t uioNum, uint8_t mapNum) {
    if(uioNum < 0 || mapNum < 0) {
		fprintf(stderr, "That is not a valid UIO device or map number\n");
		fprintf(stderr, "Check /sys/class/uio for more information about"); 
		fprintf(stderr, " the available UIO devices\n");
		exit(EXIT_FAILURE);
    }
    
    void * pwm;
    UIO * uio = UIO_MAP(uioNum, mapNum);
    pwm = uio->mapPtr;
    
    /* PWMs are initialized in the "enabled" state */
    ACCESS_REG(pwm, PWM_CTRL_OFFSET) = 0x1;

    return pwm;
}

void PWM_Enable(PWM pwm) {
    ACCESS_REG(pwm, PWM_CTRL_OFFSET) = 0x1;
}

void PWM_Disable(PWM pwm) {
    ACCESS_REG(pwm, PWM_CTRL_OFFSET) = 0x0;
}

void setPwmDuty(PWM pwm, uint8_t pwmNum, uint32_t duty) {
    ACCESS_REG(pwm, PWM_DUTY_OFFSET + (4 * (pwmNum - 1))) = duty;
}

void setPwmPeriod(PWM pwm, uint32_t period) {
    ACCESS_REG(pwm, PWM_PERIOD_OFFSET) = period;
}

uint32_t getPwmDuty(PWM pwm, uint8_t pwmNum) {
    return ACCESS_REG(pwm, PWM_DUTY_OFFSET + (4 * (pwmNum - 1)));
}

uint32_t getPwmPeriod(PWM pwm, uint8_t pwmNum) {
    return ACCESS_REG(pwm, PWM_PERIOD_OFFSET + (4 * (pwmNum - 1)));
}

uint8_t PWM_Close(PWM pwm) {
    return UIO_UNMAP(pwm);
}