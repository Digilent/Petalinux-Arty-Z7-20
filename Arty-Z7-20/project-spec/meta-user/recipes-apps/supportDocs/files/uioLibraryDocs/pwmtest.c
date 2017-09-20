#include <unistd.h>
#include "pwm-fpga.h"

void main() {
    PWM pwm = PWM_init(0, 0);

    setPwmPeriod(pwm, 50000);

    while(1) {
        for(int i = 0; i < 40000; i++) {
            setPwmDuty(pwm, 1, i);
            setPwmDuty(pwm, 3, i);
            setPwmDuty(pwm, 4, i);
            setPwmDuty(pwm, 5, i);
            usleep(1);
        }
        for(int i = 40000; i > 0; i--) {
            setPwmDuty(pwm, 1, i);
            setPwmDuty(pwm, 5, i);
            setPwmDuty(pwm, 4, i);
            setPwmDuty(pwm, 3, i);
            usleep(1);
        }

    }
}