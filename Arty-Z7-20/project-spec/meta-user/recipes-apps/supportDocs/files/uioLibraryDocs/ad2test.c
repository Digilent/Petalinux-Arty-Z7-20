#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "i2c-fpga.h"

void readADC(I2C base, uint16_t * values);
void writeRegister(I2C vm, byte slaveADX, byte reg, byte data);
void convertValues(uint16_t raw, byte * result);

const byte AD2 = 0x28;
const byte IOXP = 0x34;

#define GPO_DATA_OUT_A		0x2A
#define GPO_DATA_OUT_B		0x2B
#define GPO_OUT_MODE_A		0x2D
#define GPO_OUT_MODE_B		0x2E
#define GPIO_DIRECTION_A	0x30
#define GPIO_DIRECTION_B	0x31

int main() {
	I2C base = I2C_init(1, 1);
	byte config = 0x14;
	byte GPIO_Mode[3] = {GPO_OUT_MODE_A, 0xFF, 0xFF};
	byte GPIO_Direction[3] = {GPIO_DIRECTION_A, 0xFF, 0xFF};
	uint16_t values[4] = {0, 0, 0 , 0};

	I2C_WriteWithStop(base, AD2, &config, 1);
	I2C_WriteWithStop(base, IOXP, GPIO_Mode, 3);
	I2C_WriteWithStop(base, IOXP, GPIO_Direction, 3);

	usleep(1000); // Must wait some amount of time to let values start to populate ADC
	byte result[2];
	while(1) {
		result[0] = 0;
		result[1] = 0;
		readADC(base, values);
		float volts[4];
		for (int i = 0; i < 4; i++) {
			volts[i] = (3.3 / (float)4095) * (float)values[i];
		}
		// for(int i = 0; i < 4; i++) {
		// 	printf("Channel%d: %.4fv\t", i,  volts[i]);
		// }
		printf("Channel0: %.4fv\t\t\t\t", volts[0]);
		printf("\n");
		convertValues(values[0], result);
		writeRegister(base, IOXP, GPO_DATA_OUT_A, result[0]);
		writeRegister(base, IOXP, GPO_DATA_OUT_B, result[1]);
		usleep(25000);
	}
}

void readADC(I2C base, uint16_t * values) {
	byte temp[2] = {0, 0};
	I2C_Read(base, AD2, temp, 2);
	byte channelID = (temp[0] & 0x30) >> 4;
	values[channelID] = (((temp[0] & 0x0F) << 8) | temp[1]);
}

void writeRegister(I2C vm, byte slaveADX, byte reg, byte data) {
	byte packet[2] = {reg, data};
	I2C_WriteWithStop(vm, slaveADX, packet, 2);
}

void convertValues(uint16_t raw, byte * result) {
	raw = (raw << 2) / 3; //convert to 16 bit resolution

	//result[0] will be for A   result[1] will be for B
	uint16_t newVal = 0;
	
	for(int i = 0; i < 16; i++) {
		newVal |= (raw > (i * 341)) ? (0x0001 << i) : newVal;
	}
	
	/* A3 to A0 */
	for(int i = 0; i < 4; i++) {
		result[0] |= ((newVal & (0x0010 << i)) >> (1 + (i * 2)));
	}

	/* A7 to A4 */
	for(int i = 0; i < 4; i++) {
		result[0] |= ((newVal & (0x1000 << i)) >> (5 + (i * 2)));
	}

	/* B3 to B2 */
	for(int i = 0; i < 2; i++) {
		result[1] |= ((newVal & (0x0001 << i)) << (3 - (i * 2)));
	}

	/* B1 to B0 */
	for(int i = 0; i < 2; i++) {
		result[1] |= ((newVal & (0x0004 << i)) >> (1 + (i * 2)));
	}

	/* B7 to B4 */
	for(int i = 0; i < 4; i++) {
		result[1] |= ((newVal & (0x0100 << i)) >> (1 + (i * 2)));
	}
}

