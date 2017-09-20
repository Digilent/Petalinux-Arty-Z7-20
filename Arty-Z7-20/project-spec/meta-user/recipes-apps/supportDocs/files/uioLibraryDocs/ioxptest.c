#include <stdio.h>
#include <stdlib.h>
#include "i2c-fpga.h"
#include <unistd.h>
#include "gpio-fpga.h"

byte IOXP = 0x34;
#define GPO_DATA_OUT_A	0x2A
#define GPO_OUT_MODE_A	0x2D
#define GPIO_DIRECTION_A	0x30

void writeRegister(I2C vm, byte slaveADX, byte reg, byte data);
byte readRegister(I2C vm, byte slaveADX, byte reg);

int main(){
	I2C base = I2C_init(1, 1);
	GPIO ld4 = GPIO_init(0, 0);
	setChannelDirection(ld4, 0, 1);

	byte setup_0[3] = {GPO_OUT_MODE_A, 0xFF, 0xFF};
	byte setup_1[3] = {GPIO_DIRECTION_A, 0xFF, 0xFF};

	I2C_WriteWithStop(base, IOXP, setup_0, 3);
	I2C_WriteWithStop(base, IOXP, setup_1, 3);

	byte value_on[3] = {GPO_DATA_OUT_A, 0xFF, 0x00};
	byte value_off[3] = {GPO_DATA_OUT_A, 0x00, 0xFF};
	byte init[3] = {GPO_DATA_OUT_A, 0x00, 0x00};
	I2C_WriteWithStop(base, IOXP, init, 3);
	byte temp = 0;

	for(int j = 0; j < 16; j++) {
		setChannelValue(ld4, j, 1);
		for(int i = 0; i < 8; i++) {
			value_on[2] = 128 >> i;
			value_on[1] = 1 << i;
			I2C_WriteWithStop(base, IOXP, value_on, 3);
			temp = readRegister(base, IOXP, GPO_DATA_OUT_A);
			printf("temp variable reads %d\n", temp);
			usleep(100000);
			// I2C_WriteWithStop(base, I2C_ADX, value_off, 3);
			// usleep(200000);
		}
		for(int i = 7; i >= 0; i--) {
			value_on[2] = 128 >> i;
			value_on[1] = 1 << i;
			I2C_WriteWithStop(base, IOXP, value_on, 3);
			temp = readRegister(base, IOXP, GPO_DATA_OUT_A);
			printf("temp variable reads %d\n", temp);
			usleep(100000);
			// I2C_WriteWithStop(base, I2C_ADX, value_off, 3);
			// usleep(200000);
		}
	}
	I2C_Close(base);
	GPIO_Close(ld4);
}

void writeRegister(I2C vm, byte slaveADX, byte reg, byte data) {
	byte packet[2] = {reg, data};
	I2C_WriteWithStop(vm, slaveADX, packet, 2);
}

byte readRegister(I2C vm, byte slaveADX, byte reg) {
	byte recvBuffer;
	I2C_WriteWithoutStop(vm, slaveADX, &reg, 1);
	I2C_Read(vm, slaveADX, &recvBuffer, 1);
	return recvBuffer;
}
