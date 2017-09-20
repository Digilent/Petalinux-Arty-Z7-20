#include <unistd.h>
#include "spi-fpga.h"
#include "gpio-fpga.h"

#define DATCOM	1
#define RES		2
#define VCCEN	3
#define PMODEN	4

#define CMD_DRAWRECTANGLE                  0x22
#define CMD_FILLWINDOW                     0x26
#define CMD_DRAWLINE                       0x21
#define DISABLE_FILL    0x00
#define ENABLE_FILL     0x01

#define OLEDRGB_WIDTH                      96
#define OLEDRGB_HEIGHT                     64

void sendSingle(byte single);
void sendDouble(byte command, byte data);
void oledInit();
void OLEDrgb_DrawRectangle(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint16_t lineColor, uint8_t bFill, uint16_t fillColor);
uint16_t OLEDrgb_BuildRGB(uint8_t R,uint8_t G,uint8_t B);
uint8_t OLEDrgb_ExtractRFromRGB(uint16_t wRGB);
uint8_t OLEDrgb_ExtractGFromRGB(uint16_t wRGB);
uint8_t OLEDrgb_ExtractBFromRGB(uint16_t wRGB);
void OLEDrgb_DrawLine(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint16_t lineColor);

GPIO led;
GPIO gpio;
SPI spi;

int main() {
	gpio = GPIO_init(1, 0);
	spi = SPI_init(1, 1);
	SPI_setMode(spi, 1, 1);
	led = GPIO_init(0, 0);
	setPinMode(gpio, DATCOM, OUTPUT);
	setPinMode(gpio, RES, OUTPUT);
	setPinMode(gpio, VCCEN, OUTPUT);
	setPinMode(gpio, PMODEN, OUTPUT);
	setChannelDirection(led, 0, 1); 
	oledInit();
	byte packet[5] = {0x25, 0x00, 0x00, 95, 63};
	SPI_Transfer(spi, packet, NULL, 5);
	usleep(5000);
	OLEDrgb_DrawRectangle(0x00, 0x00, 95, 63, 0x0001, 1, 0x0001);

	sendSingle(0xAE);
	sendSingle(0xAF);
	for(int i = 0; i < OLEDRGB_WIDTH - 1; i += 16) {
		for(int j = 0; j < OLEDRGB_HEIGHT - 1; j += 16) {
			OLEDrgb_DrawRectangle(i, j, i + 8, j + 8, 0xFFFF, 1, 0x001F + i * 1000);
		}
	}
	SPI_Close(spi);
	GPIO_Close(gpio);
	for(int i = 0; i < 20; i++) {
		for(int j = 0; j < 9; j++) {
			setChannelValue(led, 3 << j, 1);
			usleep(100000);	
		}
	}
	GPIO_Close(led);

}

void oledInit() {
	digitalWrite(gpio, DATCOM, LOW);
	digitalWrite(gpio, RES, HIGH);
	digitalWrite(gpio, VCCEN, LOW);
	digitalWrite(gpio, PMODEN, HIGH);
	usleep(25000);
	digitalWrite(gpio, RES, LOW);
	usleep(100);
	digitalWrite(gpio, RES, HIGH);
	usleep(100);
	sendDouble(0xFD, 0x12);
	sendSingle(0xAE);
	sendDouble(0xA0, 0x72);
	sendDouble(0xA1, 0x00);
	sendDouble(0xA2, 0x00);
	sendSingle(0xA4);
	sendDouble(0xA8, 0x3F);
	sendDouble(0xAD, 0x8E);
	sendDouble(0xB0, 0x0B);
	sendDouble(0xB1, 0x31);
	sendDouble(0xB3, 0xF0);
	sendDouble(0x8A, 0x64);
	sendDouble(0x8B, 0x78);
	sendDouble(0x8C, 0x64);
	sendDouble(0xBB, 0x3A);
	sendDouble(0xBE, 0x3E);
	sendDouble(0x87, 0x06);
	sendDouble(0x81, 0x91);
	sendDouble(0x82, 0x50);
	sendDouble(0x83, 0x7D);
	sendSingle(0x25);
	byte packet[5] = {0x25, 0x00, 0x00, 0x5F, 0x3F};
	SPI_Transfer(spi, packet, NULL, 5);
	digitalWrite(gpio, VCCEN, HIGH);
	usleep(30000);
	sendSingle(0xAF);
	usleep(150000);


}

void sendDouble(byte command, byte data) {
	byte packet[2] = {command, data};
	SPI_Transfer(spi, packet, NULL, 2);
}

void sendSingle(byte single) {
	SPI_Transfer(spi, &single, NULL, 1);
}

void OLEDrgb_DrawRectangle(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint16_t lineColor, uint8_t bFill, uint16_t fillColor){
	//setChannelValue(gpio, 0xF, 1);
	uint8_t cmds[13];
    cmds[0] = CMD_FILLWINDOW;		//fill window
    cmds[1] = (bFill ? ENABLE_FILL: DISABLE_FILL);
    cmds[2] = CMD_DRAWRECTANGLE;	//draw rectangle
	cmds[3] = c1;					// start column
	cmds[4] = r1;					// start row
	cmds[5] = c2;					// end column
	cmds[6] = r2;					//end row

	cmds[7] = OLEDrgb_ExtractRFromRGB(lineColor);	//R
	cmds[8] = OLEDrgb_ExtractGFromRGB(lineColor);	//G
	cmds[9] = OLEDrgb_ExtractBFromRGB(lineColor);	//R

	cmds[10] = OLEDrgb_ExtractRFromRGB(fillColor);	//R
	cmds[11] = OLEDrgb_ExtractGFromRGB(fillColor);	//G
	cmds[12] = OLEDrgb_ExtractBFromRGB(fillColor);	//R

	SPI_Transfer(spi, cmds, NULL, 13);
	//setChannelValue(gpio, 0xE, 1);
}



uint16_t OLEDrgb_BuildRGB(uint8_t R,uint8_t G,uint8_t B){return ((R>>3)<<11) | ((G>>2)<<5) | (B>>3);};
uint8_t OLEDrgb_ExtractRFromRGB(uint16_t wRGB){return (uint8_t)((wRGB>>11)&0x1F);};
uint8_t OLEDrgb_ExtractGFromRGB(uint16_t wRGB){return (uint8_t)((wRGB>>5)&0x3F);};
uint8_t OLEDrgb_ExtractBFromRGB(uint16_t wRGB){return (uint8_t)(wRGB&0x1F);};

void OLEDrgb_DrawLine(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint16_t lineColor){
	uint8_t cmds[8];
	cmds[0] = CMD_DRAWLINE; 		//draw line
	cmds[1] = c1;					// start column
	cmds[2] = r1;					// start row
	cmds[3] = c2;					// end column
	cmds[4] = r2;					//end row
	cmds[5] = OLEDrgb_ExtractRFromRGB(lineColor);	//R
	cmds[6] = OLEDrgb_ExtractGFromRGB(lineColor);	//G
	cmds[7] = OLEDrgb_ExtractBFromRGB(lineColor);	//R

	SPI_Transfer(spi, cmds, NULL, 8);
}
