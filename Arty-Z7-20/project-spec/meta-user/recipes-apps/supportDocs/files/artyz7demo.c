#include "ArtyZ7.h"
#include "ChrFont0.h"
#include <signal.h>

/*******************MACRO DECLARATIONS**********************/
#define DWRITE(pin, value) ArtyDigitalWrite(pin, value)
#define DREAD(pin) ArtyDigitalRead(pin)

#define SPI_CHANNEL                         0
#define I2C_CHANNEL							0


#define DATCOM                              4
#define RES                                 5
#define VCCEN                               6
#define PMODEN                              7
#define BTN0								8
#define CMD_DRAWRECTANGLE                   0x22
#define CMD_FILLWINDOW                      0x26
#define CMD_DRAWLINE                        0x21
#define CMD_CLEAR							0x25
#define CMD_SETCOLUMNADDRESS              	0x15
#define CMD_SETROWADDRESS                 	0x75
#define DISABLE_FILL                        0x00
#define ENABLE_FILL                         0x01
#define OLEDRGB_WIDTH                       96
#define OLEDRGB_HEIGHT                      64


#define PMODCDC								0x48

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN
// Sets to refresh at 100 Hz
#define FREQ_NANO 10000000

/***************FUNCTION DECLARATIONS**********************/
void oledInit();
void drawRectangle(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint16_t lineColor, uint8_t bFill, uint16_t fillColor);
void drawPixel(uint8_t c, uint8_t r, uint16_t color);
void drawChar(uint8_t c, uint8_t r, uint16_t color, char character);
void printString(uint8_t c, uint8_t r, uint16_t color, char * str);
void OLEDrgb_Clear(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2);
void getCDCvalues(uint16_t * result);
void sendDouble(uint8_t command, uint8_t data);
void sendSingle(uint8_t single);
void timerHandler();
void cdcInit();
uint16_t OLEDrgb_BuildRGB(uint8_t R, uint8_t G, uint8_t B);
uint8_t OLEDrgb_ExtractRFromRGB(uint16_t wRGB);
uint8_t OLEDrgb_ExtractGFromRGB(uint16_t wRGB);
uint8_t OLEDrgb_ExtractBFromRGB(uint16_t wRGB);

/******************GLOBAL VARIABLES************************/
static uint16_t result[2] = {0, 0};
static uint8_t flag1 = 0;
static uint8_t flag2 = 0;
static uint16_t pwm1 = 0;
static uint16_t pwm2 = 0;
static uint16_t pwm3 = 0;
static uint16_t pwm4 = 0;

int main() {
    ArtyInit();
 
	ArtySetPinMode(14, INPUT);
	ArtySetPinMode(15, INPUT);
	ArtySetPinMode(0, OUTPUT);
	ArtySetPinMode(1, OUTPUT);
	ArtySetPinMode(BTN0, INPUT);


	ArtyI2COpenMaster(I2C_CHANNEL);
	ArtySpiOpenMaster(SPI_CHANNEL);
	oledInit();
	cdcInit();		

	ArtyPWMSetFrequency(10000);
	for(int i = 0; i < 6; i++) {
		ArtyPWMSetDuty(i, 0);
	}


	timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;
    long long freq_nanosecs = FREQ_NANO;
   	struct sigaction sa;

   	/* Set up the handler */
   	printf("Establishing handler for signal %d\n", SIG);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = timerHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIG, &sa, NULL) == -1) {
        fprintf(stderr, "Failed to establish the handler\n");
        exit(EXIT_FAILURE);
    }

    /* Create the timer */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(CLOCKID, &sev, &timerid) == -1) {
        fprintf(stderr, "Failed to create the timer\n");
        exit(EXIT_FAILURE);
    }

    /* Timing constraints */
    its.it_value.tv_sec = freq_nanosecs / 1000000000;
    its.it_value.tv_nsec = freq_nanosecs % 1000000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
	
	/* Start the timer */
	if (timer_settime(timerid, 0, &its, NULL) == -1) {
		fprintf(stderr, "Failed to start timer");
		exit(EXIT_FAILURE);
	}

	OLEDrgb_Clear(0, 0, OLEDRGB_WIDTH - 1, OLEDRGB_HEIGHT - 1);
	// printString(16, 8, 0xFF00, "Danica");
	// printString(16, 16, 0x0FF0, "Morovic");
	// printString(16, 24, 0x00FF, "Doesn't");
	// printString(16, 32, 0xFFFF, "Know what");
	// printString(16, 40, 0x00FF, "To do");
	// printString(16, 48, 0x0FF0, "With her");
	// printString(16, 56, 0xFF00, "Life");



	while(1) {

 

		if(DREAD(BTN0)) {
			break;
		}
	}
	
	OLEDrgb_Clear(0, 0, OLEDRGB_WIDTH - 1, OLEDRGB_HEIGHT - 1);
    ArtySpiCloseMaster(SPI_CHANNEL);
	ArtyI2CCloseMaster(I2C_CHANNEL);
	ArtyDeInit();
}




/**********************************************************
 * @name: oledInit()
 * @param: none
 * @pre: ArtyInit() must be called before this function is called
 *      assumes there is a valid spi channel open already
 * 
 * Runs the initialization sequence for the RGB Oled
 *********************************************************/
void oledInit() {
    // Set GPIOS to outputs
    ArtySetPinMode(DATCOM, OUTPUT);
    ArtySetPinMode(RES, OUTPUT);
    ArtySetPinMode(VCCEN, OUTPUT);
    ArtySetPinMode(PMODEN, OUTPUT);

    // Set initial GPIO 
    DWRITE(DATCOM, LOW);
    DWRITE(RES, HIGH);
    DWRITE(VCCEN, LOW);
    DWRITE(PMODEN, HIGH);
    usleep(25000);
    DWRITE(RES, LOW);
    usleep(100);
    DWRITE(RES, HIGH);
    usleep(100);

    // Send SPI commands to initialize OLED
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
	uint8_t packet[5] = {0x25, 0x00, 0x00, 0x5F, 0x3F};
	ArtySpiTransfer(SPI_CHANNEL, packet, NULL, 5);
	DWRITE(VCCEN, HIGH);
	usleep(30000);
	sendSingle(0xAF);uint8_t ch1Config[2] = {0x0B, 0xCB};
	ArtyI2CWrite(I2C_CHANNEL, PMODCDC, ch1Config, 2);
	usleep(150000);
}

void drawRectangle(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint16_t lineColor, uint8_t bFill, uint16_t fillColor){

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
	cmds[9] = OLEDrgb_ExtractBFromRGB(lineColor);	//R		OLEDrgb_Clear();

	cmds[10] = OLEDrgb_ExtractRFromRGB(fillColor);	//R
	cmds[11] = OLEDrgb_ExtractGFromRGB(fillColor);	//G
	cmds[12] = OLEDrgb_ExtractBFromRGB(fillColor);	//R

	ArtySpiTransfer(SPI_CHANNEL, cmds, NULL, 13);

}

void drawPixel(uint8_t c, uint8_t r, uint16_t color) {
	uint8_t cmds[6];
	uint8_t data[2];

	cmds[0] = CMD_SETCOLUMNADDRESS;
	cmds[1] = c;
	cmds[2] = OLEDRGB_WIDTH - 1;
	cmds[3] = CMD_SETROWADDRESS;
	cmds[4] = r;
	cmds[5] = OLEDRGB_HEIGHT - 1;
	
	data[0] = color >> 8;
	data[1] = color;

	ArtySpiTransfer(SPI_CHANNEL, cmds, NULL, 6);
	DWRITE(DATCOM, HIGH);
	ArtySpiTransfer(SPI_CHANNEL, data, NULL, 2);
	DWRITE(DATCOM, LOW);
	
}

void drawChar(uint8_t c, uint8_t r, uint16_t color, char character) {
	uint16_t index = (character - 0x20) * 8;
	for(int i = 0; i < 8; i++) {
		uint8_t * cur = rgbOledRgbFont0 + index + i;
		for(int j = 0; j < 8; j++) {
			if((*cur) >> j & 0x01) {
				drawPixel(i + c, j + r, color);
			} else {
				drawPixel(i + c, j + r, 0x0000);
			}
		}
	}
}

void printString(uint8_t c, uint8_t r, uint16_t color, char * str) {
	uint8_t count = 0;
	while(*str != '\0') {
		drawChar(c + count, r, color, *str);
		count += 8;
		str++;
	}
}

void OLEDrgb_Clear(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2) {
	uint8_t cmds[5];
	cmds[0] = CMD_CLEAR;
	cmds[1] = c1;
	cmds[2] = r1;
	cmds[3] = c2;
	cmds[4] = r2;
	ArtySpiTransfer(SPI_CHANNEL, cmds, NULL, 5);
	usleep(5000);
}

void sendDouble(uint8_t command, uint8_t data) {
    uint8_t packet[2] = {command, data};
    ArtySpiTransfer(SPI_CHANNEL, packet, NULL, 2);
}

void sendSingle(uint8_t single) {
    ArtySpiTransfer(SPI_CHANNEL, &single, NULL, 1);
}

uint16_t OLEDrgb_BuildRGB(uint8_t R, uint8_t G, uint8_t B) {
    return ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3);
}

uint8_t OLEDrgb_ExtractRFromRGB(uint16_t wRGB) {
    return (uint8_t) ((wRGB >> 11) & 0x1F);
}

uint8_t OLEDrgb_ExtractGFromRGB(uint16_t wRGB) {
    return (uint8_t) ((wRGB >> 5) & 0x3F);
}

uint8_t OLEDrgb_ExtractBFromRGB(uint16_t wRGB) {
    return (uint8_t) (wRGB & 0x1F);
}

void getCDCvalues(uint16_t * result) {
	uint8_t regPtr[4] = {0x01, 0x02, 0x03, 0x04};
	uint8_t data[4] = {0, 0, 0, 0};
	for(int i = 0; i < 4; i++) {
		ArtyI2CWrite(I2C_CHANNEL, PMODCDC, regPtr + i, 1);
		ArtyI2CRead(I2C_CHANNEL, PMODCDC, data + i, 4);
	}
	result[0] = (data[0] << 8) | (data[1] & 0xF0);
	result[1] = (data[2] << 8) | (data[3] & 0xF0);

}

void timerHandler() {
	if(DREAD(14)) {
		flag1 = 1;
		DWRITE(0, HIGH);
	} else {
		flag1 = 0;
		DWRITE(0, LOW);
	}
	if(DREAD(15)) {
		flag2 = 1;
		DWRITE(1, HIGH);
	} else {
		flag2 = 0;
		DWRITE(1, LOW);
	}
	
	if(flag1) {

		pwm1 += 25;
		if(pwm1 > 2500) {
			pwm3 += 25;
		}
		printString(32, 16, 0x07F0, "BTN 1");
	} else {
		pwm1 = 0;
		pwm3 = 0;
		OLEDrgb_Clear(0, 0, OLEDRGB_WIDTH - 1, 32);
	}
	if(flag2) {
		pwm2 += 25;
		if(pwm2 > 2500) {
			pwm4 += 25;
		}
		printString(32, 40, 0x00FF, "BTN 2");
	} else {
		pwm2 = 0;
		pwm4 = 0;
		OLEDrgb_Clear(0, 32, OLEDRGB_WIDTH -1, OLEDRGB_HEIGHT - 1);
	}

	ArtyPWMSetDuty(4, pwm1);
	ArtyPWMSetDuty(5, pwm3);
	ArtyPWMSetDuty(0, pwm2);
	ArtyPWMSetDuty(2, pwm4);

	//getCDCvalues(result);
	// printf("Channel 1: %d\t\t", result[1]);
	// printf("Channel 2: %d\n", result[0]);

}

void cdcInit() {
	uint8_t config[2] = {0x0F, 0x19};
	ArtyI2CWrite(I2C_CHANNEL, PMODCDC, config, 2);

	uint8_t ch1Config[2] = {0x0B, 0xCB};
	ArtyI2CWrite(I2C_CHANNEL, PMODCDC, ch1Config, 2);

	uint8_t ch2Config[2] = {0x0E, 0xCB};
	ArtyI2CWrite(I2C_CHANNEL, PMODCDC, ch2Config, 2);

	uint8_t capdac1[2] = {0x11, 0xFF};
	uint8_t capdac2[2] = {0x12, 0xFF};
	ArtyI2CWrite(I2C_CHANNEL, PMODCDC, capdac1, 2);
	ArtyI2CWrite(I2C_CHANNEL, PMODCDC, capdac2, 2);
}