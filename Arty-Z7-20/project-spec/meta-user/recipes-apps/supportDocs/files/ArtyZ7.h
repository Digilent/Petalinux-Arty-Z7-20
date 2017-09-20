#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <termios.h>	
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include "gpio-fpga.h"
#include "pwm-fpga.h"

/* Macro for calculating Spi channel numbers */
#define GET_SPI_CHANNEL(channel) 32766 - channel

/* Base File Port defines */
#define I2C_BASE_PORT       "/dev/i2c-"
#define UART_BASE_PORT      "/dev/ttyS"
#define UIO_BASE            "/sys/class/uio/"
#define MAX_DEVICE_NUM_COMM     10

// Operational masks for SPI
#define MSBFIRST                0
#define LSBFIRST                1

typedef struct SPI {
    int spiFD;              // File Descriptor for the SPI master
    unsigned long maxSpeed; // Max transfer speed in Hz
    uint8_t cs;             // Current chip select configuration
    uint8_t mode;           // current SPI_MODE
    uint8_t bitOrder;       // MSB first or LSB first
} SPIdata;

typedef struct I2C {
    int i2cFD;              // File Descriptor for the I2C master
} I2Cdata;

typedef struct UART {
    int uartFD;             // File Descriptor for the UART channel
} UARTdata;

typedef struct GPIOinfo {
    GPIO gpioDevice;        // GPIO data type from gpio-fpga.h
    uint8_t uioNum;         // The UIO device number of the GPIOS
    uint8_t isDual;         // 1 if dual channel, 0 if single channel
    uint8_t channel1Width;  // Bit width of channel 1
    uint8_t channel2Width;  // Bit width of channel 2
} GPIOdata;

/***************Library Entrance and Exits****************/
int ArtyInit();
int ArtyDeInit();

/*******************GPIO functionality**********************/
int ArtyDigitalWrite(uint8_t pin, uint8_t value);
int ArtyDigitalRead(uint8_t pin);
int ArtySetPinMode(uint8_t pin, uint8_t mode);
int ArtyGetDIOChannels(uint8_t * numChannels, uint8_t * channelArray);

/*******************PWM functionality**********************/
int ArtyPWMenable();
int ArtyPWMSetFrequency(unsigned long nano);
int ArtyPWMSetDuty(uint8_t channel, unsigned long nano);
int ArtyPWMdisable();

/*******************SPI functionality**********************/
int ArtySpiOpenMaster(uint8_t channel);
int ArtySpiSetBitOrder(uint8_t channel, uint8_t bitOrder);
int ArtySpiSetMode(uint8_t channel, uint8_t mode);
int ArtySpiSetMaxSpeed(uint8_t channel, unsigned long speed);
int ArtySpiTransfer(uint8_t channel, uint8_t * tx_buffer, uint8_t * rx_buffer, uint8_t numBytes);
unsigned int ArtyGetSpiTransferSpeed(uint8_t channel);
int ArtySpiCloseMaster(uint8_t channel);

/*******************I2C functionality**********************/
int ArtyI2COpenMaster(uint8_t channel);
int ArtyI2CWrite(uint8_t channel, uint8_t slaveAdx, uint8_t * tx_buffer, uint8_t numBytes);
int ArtyI2CRead(uint8_t channel, uint8_t slaveAdx, uint8_t * rx_buffer, uint8_t numBytes);
int ArtyI2CCloseMaster(uint8_t channel);

/*******************UART functionality**********************/
int ArtyUartOpen(uint8_t channel);
int ArtyUartSetBaudRate(uint8_t channel, unsigned long baud);
int ArtyUartGetBytesAvailable(uint8_t channel, uint8_t * numBytes);
int ArtyUartRead(uint8_t channel, uint8_t numBytes, uint8_t * rxbuffer, uint8_t * numBytesRead);
int ArtyUartWrite(uint8_t channel, uint8_t numBytes, uint8_t * txbuffer);
int ArtyUartClose(uint8_t channel);


