#include "ArtyZ7.h"

/******************Function Declarations*****************/
static void populateGpioData();
static unsigned int ArtyGetSpiSpeed(uint8_t channel);
static uint8_t reverseBits(uint8_t b);


/*********************************************************
 *              The base data structures
 ********************************************************/
SPIdata * spiDevices[MAX_DEVICE_NUM_COMM];    
I2Cdata * i2cDevices[MAX_DEVICE_NUM_COMM];
UARTdata * uartDevices[MAX_DEVICE_NUM_COMM];
PWM pwmDevice = NULL;
GPIOdata * gpioMeta = NULL;



/*********************************************************
 * @Name:   ArtyInit()
 * @pre:    Notes on usage and constraints is found in
 *          'NotesandKnownIssues.md'
 * @param:  None
 * @return: 0 on success, 1 on failure
 *
 * Initializes the underlying data structures for further use
 * Also, initializes the GPIO and PWM functionality if they
 * are already present in the device-tree
 *
 ********************************************************/
int ArtyInit() {
    for(int i = 0; i < MAX_DEVICE_NUM_COMM; i++) {
        spiDevices[i] = NULL;
        i2cDevices[i] = NULL;
        uartDevices[i] = NULL;
    }

    int gpioNum = -1;
    int pwmNum = -1;
    for(int i = 0; i < 2; i++) {
        char buf[50];
        char c[4];
        sprintf(buf, "%s%s%d%s", UIO_BASE, "uio", i, "/name");
        FILE * uios;
        uios = fopen(buf, "r");
        if((NULL == uios) && (i == 0)) {
            perror("");
            fprintf(stderr, "There are no PWM or GPIO devices\n");
            pwmDevice = NULL;
            gpioMeta = NULL;
            break;
        } else if (NULL == uios) {
            break; 
        }else {
            fgets(c, 4, uios);
            int test = strcmp(c, "PWM");
            if(test == 0) {
                pwmNum = i;
            } else {
                gpioNum = i;
            }
        }
    }
    if(pwmNum > -1) {
        pwmDevice = PWM_init(pwmNum, 0);
        fprintf(stderr, "PWM Device mapped as uio%d\n", pwmNum); 
    }
    if(gpioNum > -1) {
        gpioMeta = (GPIOdata *) malloc(sizeof(GPIOdata));
        (*gpioMeta).gpioDevice = GPIO_init(gpioNum, 0);
        (*gpioMeta).uioNum = gpioNum;
        populateGpioData();
        fprintf(stderr, "GPIO Device mapped as uio%d\n", gpioNum);
    }
    
    return 0;
}


/*********************************************************
 * @Name:   ArtyDeInit()
 * @pre:    Notes on usage and constraints is found in
 *          'NotesandKnownIssues.md'
 * @param:  None
 * @return: 0 always
 *
 * Closes any open PWM or GPIO devices that had earlier been
 * instantiated
 *
 ********************************************************/
int ArtyDeInit() {
    if(NULL != pwmDevice) {
        PWM_Close(pwmDevice);
        pwmDevice = NULL;
    }
    if(NULL != gpioMeta) {
        GPIO_Close((*gpioMeta).gpioDevice);
        free(gpioMeta);
        gpioMeta = NULL;
    }

    return 0;
}

static void populateGpioData() {
    char filePath[100];
    char addressString[11];
    /************************************************
    *   Get path information from UIO directory
    ************************************************/
    sprintf(filePath, "%suio%d/maps/map0/addr", UIO_BASE, (*gpioMeta).uioNum);
    FILE * addressPath = fopen(filePath, "r");
    if(NULL == addressPath) {
        fprintf(stderr, "uioNum = %d\n", (*gpioMeta).uioNum);
        fprintf(stderr, "The specified address file for UIO does not exist\n");
        return;
    }
    if(NULL == fgets(addressString, 11, addressPath)) {
        fprintf(stderr, "Unable to read from address UIO file\n");
        return;
    }

    char * adx = &(addressString[2]);
    char dtPath[50];
    sprintf(dtPath, "/proc/device-tree/gpio@%s/", adx);
    char isDualPath[50];
    char chan1Path[50];
    sprintf(isDualPath, "%sxlnx,is-dual", dtPath);
    sprintf(chan1Path, "%sxlnx,gpio-width", dtPath);
    

    /************************************************
    *   Get dual channel information from device-tree
    ************************************************/
    uint8_t dualInfo[5];
    FILE * dualFile = fopen(isDualPath, "r");
    if(NULL == dualFile) {
        fprintf(stderr, "Unable to open Dual channel file\n");
        return;
    }
    if(NULL == fgets(dualInfo, 5, dualFile)) {
        fprintf(stderr, "Unable to read dual Channel info\n");
        return;
    }
    (*gpioMeta).isDual = dualInfo[3];

    
    /************************************************
    *   Get channel 1 information from device-tree
    ************************************************/
    uint8_t chan1Info[5];
    FILE * chan1File = fopen(chan1Path, "r");
    if(NULL == chan1File) {
        fprintf(stderr, "Unable to open channel 1 file\n");
        return;
    }
    if(NULL == fgets(chan1Info, 5, chan1File)) {
        fprintf(stderr, "Unable to read channel 1 info\n");
        return;
    }
    (*gpioMeta).channel1Width = chan1Info[3];

    
    /************************************************
    *   Get channel 2 information from device-tree
    ************************************************/
    if((*gpioMeta).isDual) {
        char chan2Path[50];
        sprintf(chan2Path, "%sxlnx,gpio2-width", dtPath);
        uint8_t chan2Info[5];
        FILE * chan2File = fopen(chan2Path, "r");
        if(NULL == chan1File) {
            fprintf(stderr, "Unable to open channel 2 file\n");
            return;
        }
        if(NULL == fgets(chan2Info, 5, chan2File)) {
            fprintf(stderr, "Unable to read channel 2 info\n");
            return;
        }
        (*gpioMeta).channel2Width = chan2Info[3];
    } else {
        (*gpioMeta).channel2Width = 0;
    }

    /*
    fprintf(stderr, "Is-Dual: %d\n", (*gpioMeta).isDual);
    fprintf(stderr, "CH1-Width: %d\n", (*gpioMeta).channel1Width);
    fprintf(stderr, "CH2-Width: %d\n", (*gpioMeta).channel2Width);
    */
}

/*********************************************************
 * @Name:   ArtySetPinMode()
 * @pre:    Assumes a gpio device has already been instantiated
 *          and that there are valid gpio channels to work on
 *          Does not check to see if the channels are valid
 * @param:  uint8_t pin     The pin to work on
 *          uint8_t mode    the mode to set it to such as
 *                          "OUTPUT" or "INPUT"
 * @return: 0 on success, 1 on failure
 *
 * Sets the mode of a specific GPIO pin
 *
 ********************************************************/
int ArtySetPinMode(uint8_t pin, uint8_t mode) {
    if(NULL == gpioMeta || (*gpioMeta).gpioDevice == NULL) {
        fprintf(stderr, "There is no GPIO device\n");
        return 1;
    } else {
        if(pin > ((*gpioMeta).channel1Width - 1)) {
            setPinMode((*gpioMeta).gpioDevice, 2, pin - (*gpioMeta).channel1Width + 1, mode);
        } else {
            setPinMode((*gpioMeta).gpioDevice, 1, pin + 1, mode);
        }
    }

    return 0;
}

/*********************************************************
 * @Name:   ArtyDigitalWrite()
 * @pre:    Assumes a gpio device has already been instantiated
 *          and that there are valid gpio channels to work on
 *          Does not check to see if the channels are valid
 *          Does not check if pin mode has been set to in our out
 *          Just attempts to write the value
 * @param:  uint8_t pin     The pin to work on
 *          uint8_t value   The value to set the pin to such as
 *                          "HIGH" or "LOW". Any non-zero values
 *                          are taken to mean the same as "HIGH"
 * @return: 0 on success, 1 on failure
 *
 * Writes 'value' to GPIO 'pin'
 *
 ********************************************************/
int ArtyDigitalWrite(uint8_t pin, uint8_t value) {
    if(NULL == gpioMeta || (*gpioMeta).gpioDevice == NULL) {
        fprintf(stderr, "There is no GPIO device\n");
        return 1;
    } else {
        if (pin > ((*gpioMeta).channel1Width - 1)) {
            digitalWrite((*gpioMeta).gpioDevice, 2, (pin - (*gpioMeta).channel1Width) + 1, value);
        } else {
            digitalWrite((*gpioMeta).gpioDevice, 1, pin + 1, value);
        }
    }

    return 0;
}

/*********************************************************
 * @Name:   ArtyDigitalRead()
 * @pre:    Assumes a gpio device has already been instantiated
 *          and that there are valid gpio channels to work on
 *          Does not check to see if the channels are valid
 *          Does not check to see if the GPIO has been set to
 *          Input. Just attempts to read the value
 * @param:  uint8_t pin     The pin to read
 *
 * @return: the pin value on success, -1 on failure
 *
 * Reads the value from the GPIO 'pin'
 *
 ********************************************************/
int ArtyDigitalRead(uint8_t pin) {
    if(NULL == gpioMeta || (*gpioMeta).gpioDevice == NULL) {
        fprintf(stderr, "There is no GPIO device\n");
        return -1;
    } else {
        if (pin > ((*gpioMeta).channel1Width - 1)) {
            return digitalRead((*gpioMeta).gpioDevice, 2, pin - (*gpioMeta).channel1Width + 1);
        } else {
            return digitalRead((*gpioMeta).gpioDevice, 1, pin + 1);
        }
    }

    return 0;
}

/*********************************************************
 * @Name:   ArtyGetDIOChannels()
 * @pre:    Assumes a gpio device has already been instantiated
 *          and that there are valid gpio channels to work on
 *          Does not check to see if the channels are valid
 *          Does not check to see if the GPIO has been set to
 *          Input. Just attempts to read the value
 * @param:  uint8_t * numChannels   pointer to the location
 *                                  where the number of channels will
 *                                  stored after function call
 *          uint8_t * channelArray  pointer to the location where
 *                                  an array of these valid channels
 *                                  will be stored
 *
 * @return: 0 on success, 1 on failure
 *
 * Places the valid number of GPIO channels at *numChannels
 * Creates an array containing these valid channels at *channelArray
 *
 ********************************************************/
int ArtyGetDIOChannels(uint8_t * numChannels, uint8_t * channelArray) {
    if(NULL == gpioMeta || (*gpioMeta).gpioDevice == NULL) {
        fprintf(stderr, "There is no GPIO device\n");
        return 1;
    }

    *numChannels = (*gpioMeta).channel1Width + (*gpioMeta).channel2Width;
    uint8_t * index = channelArray;
    for(int i = 0; i < *numChannels; i++) {
        *index = i;
        index++;
    }
    return 0;
}

/*********************************************************
 * @Name:   ArtyPWMenable()
 * @pre:    Assumes a PWM device has already been instantiated
 *
 * @param:  none
 *
 * @return: 0 on success, 1 on failure
 *
 * Enables the PWM channels
 *
 ********************************************************/
int ArtyPWMenable() {
    if(NULL == pwmDevice) {
        fprintf(stderr, "There is no PWM device to enable\n");
        return 1;
    } else {
        PWM_Enable(pwmDevice);
    }
    
    return 0;
}

/*********************************************************
 * @Name:   ArtyPWMdisable()
 * @pre:    Assumes a PWM device has already been instantiated
 *
 * @param:  none
 *
 * @return: 0 on success, 1 on failure
 *
 * Disables the PWM channels
 *
 ********************************************************/
int ArtyPWMdisable() {
    if(NULL == pwmDevice) {
        fprintf(stderr, "There is no PWM device to disable\n");
        return 1;
    } else {
        PWM_Disable(pwmDevice);
    }

    return 0;
}


/*********************************************************
 * @Name:   ArtyPWMSetFrequency()
 * @pre:    Assumes a PWM device has already been instantiated
 *          and the number of channels has been set in HW design
 *          Assumes a 100Mhz PL clock. Function will still work
 *          with a different clock rate, but does not set in nanoseconds
 * @param:  unsigned long nano      number of nanoseconds to set the
 *                                   frequency of the PWM channels to
 *
 * @return: 0 on success, 1 on failure
 *
 * Sets the frequency, in nanoseconds, of all the PWM channels
 *
 ********************************************************/
int ArtyPWMSetFrequency(unsigned long nano) {
    if(NULL == pwmDevice) {
        fprintf(stderr, "There is no PWM device\n");
        return 1;
    } else {
        setPwmPeriod(pwmDevice, nano / 10);
    }

    return 0;
}

/*********************************************************
 * @Name:   ArtyPWMSetDuty()
 * @pre:    Assumes a PWM device has already been instantiated
 *          and the number of channels has been set in HW design
 *          Assumes a 100Mhz PL clock. Function will still work
 *          with a different clock rate, but does not set in nanoseconds
 * @param:  uint8_t channel         the PWM channel to work on -- zero-indexed
 *          unsigned long nano      number of nanoseconds to set the
 *                                   frequency of the PWM channels to
 *
 * @return: 0 on success, 1 on failure
 *
 * Sets the duty cycle or "time on", in nanoseconds,
 * of a specific PWM channel
 *
 ********************************************************/
int ArtyPWMSetDuty(uint8_t channel, unsigned long nano) {
    if(NULL == pwmDevice) {
        fprintf(stderr, "There is no PWM device\n");
        return 1;
    } else {
        setPwmDuty(pwmDevice, channel + 1, nano / 10);
    }

    return 0;
}


/*********************************************************
 * @Name:   ArtySpiOpenMaster()
 * @pre:    Assumes spi hardware is present in hardware and in the device tree
 * @param:  uint8_t channel         The SPI channel to open
 *
 * @return: 0 on success, 1 on failure
 *
 * Opens and prepares a spi channel for transfer with the following settings:
 *  Sets SPI channel to SPI_MODE_0
 *  Sets MaxSpeed to the speed set in hardware design
 *  Sets CS to 0
 *  Sets MSB mode to MSB
 *  Sets bit transfer width to 8 
 *
 ********************************************************/
int ArtySpiOpenMaster(uint8_t channel) {
    if(NULL != spiDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A Spi device already exists on this channel: %d\n", channel);
        return 1;
    }

    char port[50];
    sprintf(port, "/dev/spidev%d.0", GET_SPI_CHANNEL(channel));

    spiDevices[channel] = (SPIdata *) malloc(sizeof(SPIdata));
    if(((*spiDevices[channel]).spiFD = open(port, O_RDWR)) < 0) {
        perror("Failed to open SPI port: ");
        free(spiDevices[channel]);
        spiDevices[channel] = NULL;
        return 1;
    }

    uint8_t bits = 8;
    int status;
    if((status = ioctl((*spiDevices[channel]).spiFD, SPI_IOC_WR_BITS_PER_WORD, &bits)) < 0) {
        perror("Failed to set packet size on writes: ");
        return 1;
	}
	if((status = ioctl((*spiDevices[channel]).spiFD, SPI_IOC_RD_BITS_PER_WORD, &bits)) < 0) {
        perror("Failed to set packet size on reads: ");
        return 1;
	}

    ArtySpiSetBitOrder(channel, MSBFIRST);  // Default to MSB first
    ArtySpiSetMode(channel, SPI_MODE_0);    // Default to Spi Mode 0
    (*spiDevices[channel]).maxSpeed = ArtyGetSpiSpeed(channel);
    ArtySpiSetMaxSpeed(channel, 0);
    (*spiDevices[channel]).cs = 0;          // Default to CS 0
    return 0;

}

/*********************************************************
 * @Name:   ArtySpiSetBitOrder()
 * @pre:    Assumes the spi channel is open and valid
 * @param:  uint8_t channel         The SPI channel to open
 *          uint8_t bitOrder        either MSBFIRST or LSBFIRST
 *
 * @return: 0 on success, 1 on failure
 *
 * Sets Bit order of SPI Transfers, either MSB or LSB first
 *
 ********************************************************/
int ArtySpiSetBitOrder(uint8_t channel, uint8_t bitOrder) {
    if(NULL == spiDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A Spi device has not been opened on this channel: %d\n", channel);
        return 1;
    }
    
    (*spiDevices[channel]).bitOrder = bitOrder;
    return 0;
}

/*********************************************************
 * @Name:   ArtySpiSetBitOrder()
 * @pre:    Assumes the spi channel is open and valid
 * @param:  uint8_t channel     The SPI channel to open
 *          uint8_t mode        SPIDEV provides some great macros for this
 *                               SPI_MODE_0      SPI_MODE_2
 *                               SPI_MODE_1      SPI_MODE_3 
 *                              are the recommended arguments to use for this parameter
 *
 * @return: 0 on success, 1 on failure
 *
 * Sets the SPI mode to be used for transfers
 *
 ********************************************************/
int ArtySpiSetMode(uint8_t channel, uint8_t mode) {
    if(NULL == spiDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A Spi device has not been opened on this channel: %d\n", channel);
        return 1;
    }

    int status;
    if((status = ioctl((*spiDevices[channel]).spiFD, SPI_IOC_WR_MODE, &mode)) < 0) {
        perror("Failed to Set SPI Write Mode: ");
        return 1;
	}
	if((status = ioctl((*spiDevices[channel]).spiFD, SPI_IOC_RD_MODE, &mode)) < 0) {
        perror("Failed to Set SPI Read Mode: ");
        return 1;
	}
    (*spiDevices[channel]).mode = mode;

    return 0;

}

/*********************************************************
 * @Name:   ArtySpiSetMaxSpeed()
 * @pre:    Assumes the spi channel is open and valid
 * @param:  uint8_t channel     The SPI channel to open
 *          unsigned long speed The desired maxspeed for SPI transfers
 *
 * @return: 0 on success, 1 on failure
 *
 * Sets MAX transfer speed of SPI transfers
 * It should be noted that this function will do nothing for
 * The axi-quad-spi IP provided by Vivado as SPI transfers will
 * always take place at the rate specified by ext-spi-clock
 * in hardware design
 * 
 ********************************************************/
int ArtySpiSetMaxSpeed(uint8_t channel, unsigned long speed) {
    if(NULL == spiDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A Spi device has not been opened on this channel: %d\n", channel);
        return 1;
    }

    int status;
    if((status = ioctl((*spiDevices[channel]).spiFD, SPI_IOC_WR_MAX_SPEED_HZ, &(*spiDevices[channel]).maxSpeed)) < 0) {
        perror("Failed to Set SPI Write Speed: ");
        return 1;
	}
	if((status = ioctl((*spiDevices[channel]).spiFD, SPI_IOC_RD_MAX_SPEED_HZ, &(*spiDevices[channel]).maxSpeed)) < 0) {
        perror("Failed to Set SPI Read Speed: ");
        return 1;
	}

    return 0;
}

/*********************************************************
 * @Name:   ArtySpiGetSpiTransferSpeed()
 * @pre:    Assumes the spi channel is open and valid
 * @param:  uint8_t channel     The SPI channel to get speeds of
 *
 * @return: unsigned int    The value of the spi-max-frequency field
 *                           set in the device-tree spidev slave node
 *                           returns 1 on failure
 *
 * Returns the value specified by the spi-max-frequency field
 * in the device tree for this specific spi channel
 * 
 ********************************************************/
unsigned int ArtyGetSpiTransferSpeed(uint8_t channel) {
    if(NULL == spiDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A Spi device has not been opened on this channel: %d\n", channel);
        return 1;
    } else {
        return (*spiDevices[channel]).maxSpeed;
    }
    
    return 0;
}

/*********************************************************
 * @Name:   ArtySpiTransfer()
 * @pre:    Assumes the spi channel is open and valid
 * @param:  uint8_t channel     The SPI channel to transfer on
 *          uint8_t * tx_buffer The location where data to be transferred is stored
 *          uint8_t * rx_buffer The location where data to be received should be stored
 *          uint8_t numBytes    The number of bytes to transfer/receive from the salve
 *
 * @return: number of bytes transferred on success, -1 on failure
 *
 * Transfer numBytes from tx_buffer and receives numBytes into rx_buffer
 * Either one of these can be set to NULL if transferred or received data is to
 * be discarded
 * 
 ********************************************************/
int ArtySpiTransfer(uint8_t channel, uint8_t * tx_buffer, uint8_t * rx_buffer, uint8_t numBytes) {
    if(NULL == spiDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A Spi device has not been opened on this channel: %d\n", channel);
        return -1;
    }

    if((*spiDevices[channel]).bitOrder == LSBFIRST) {
        for(int i = 0; i < numBytes; i++) {
            *(tx_buffer + i) = reverseBits(*(tx_buffer + i));
        }
    }

    struct spi_ioc_transfer transfer = {
        .tx_buf = tx_buffer,
        .rx_buf = rx_buffer,
        .len = numBytes,
        .delay_usecs = 0,
        .speed_hz = 0,
        .bits_per_word = 0,
    };

    int status;
    if ((status = ioctl((*spiDevices[channel]).spiFD, SPI_IOC_MESSAGE(1), &transfer)) < 0) {
        perror("FAILED TO SEND SPI DATA: ");
        return -1;
    }
    
    return status;
}

/* Returns NULL if the file does not exist */
/* Returns the spi transfer speed set in the FPGA hardware */
static unsigned int ArtyGetSpiSpeed(uint8_t channel) {
    if(NULL == spiDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A Spi device has not been opened on this channel: %d\n", channel);
        return 1;
    }
    
    unsigned int result = 0;
    char filePath[100];
    sprintf(filePath, "/sys/class/spi_master/spi%d/of_node/spidev@0/spi-max-frequency", GET_SPI_CHANNEL(channel));
    FILE * maxFreq = fopen(filePath, "r");
    if(NULL == maxFreq) {
        fprintf(stderr, "The spi-max-frequency file does not exist\n");
        return 1;
    }
    unsigned char fileData[5]; // 32 bit value to be read....4 locations plus terminating character
    fgets(fileData, 5, maxFreq);
    unsigned char * index = fileData;
    for(int i = 3; i > -1; i--) {
        unsigned int tempValue = *index; // Get the data stored and convert to 32 bit value
        for(int j = 1; j <= i; j++) {
            tempValue *= 256; // Do the hexadecimal to decimal conversion
        }
        result += tempValue;
        index++;
    }
    return result;
}

/*********************************************************
 * @Name:   ArtySpiCloseMaster()
 * @pre:    Assumes the spi channel is open and valid
 * @param:  uint8_t channel     The SPI channel to close
 *
 * @return: 0 on success, 1 on failure
 *
 * Closes the specified SPI channel
 * 
 ********************************************************/
int ArtySpiCloseMaster(uint8_t channel) {
    if(NULL == spiDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A Spi device does not exist on this channel: %d\n", channel);
        return 1;
    }

    close((*spiDevices[channel]).spiFD);
    free(spiDevices[channel]);
    spiDevices[channel] = NULL;
    return 1;
}

/*********************************************************
 * @Name:   ArtyI2COpenMaster()
 * @pre:    Assumes that I2C hardware exists and has been
 *          loaded into the live device tree
 * @param:  uint8_t channel     The I2C channel to open
 *
 * @return: 0 on success, 1 on failure
 *
 * Opens the specified I2C hardware at the specified channel
 * 
 ********************************************************/
int ArtyI2COpenMaster(uint8_t channel) {
    if(NULL != i2cDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "An I2C device already exists on this channel: %d\n", channel);
        return 1;
    }

    char port[50];
    sprintf(port, "%s%d", I2C_BASE_PORT, channel);
    i2cDevices[channel] = (I2Cdata *)malloc(sizeof(I2Cdata));

    if(((*i2cDevices[channel]).i2cFD = open(port, O_RDWR)) < 0) {
        perror("Failed to open I2C port: ");
        free(i2cDevices[channel]);
        i2cDevices[channel] = NULL;
        return 1;
    }

    return 0;
}

/*********************************************************
 * @Name:   ArtyI2CWrite()
 * @pre:    Assumes that the I2C channel has already been opened
 * @param:  uint8_t channel     The I2C channel to open
 *          uint8_t slaveAdx    The I2C slave address to write to
 *          uint8_t * tx_buffer The location of data to be written
 *          uint8_t numBytes    The number of bytes to write from tx_buffer
 *
 * @return: 0 if all bytes are written, 1 otherwise
 *
 * Writes numBytes from tx_buffer to I2C device at slaveAdx
 * 
 ********************************************************/
int ArtyI2CWrite(uint8_t channel, uint8_t slaveAdx, uint8_t * tx_buffer, uint8_t numBytes) {
    if(NULL == i2cDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "An I2C device does not exist on this channel: %d\n", channel);
        return 1;
    }

    if(ioctl((*i2cDevices[channel]).i2cFD, I2C_SLAVE, slaveAdx) < 0) {
        perror("Failed to register slave on I2C bus: ");
        return 1;
    }

    if(write((*i2cDevices[channel]).i2cFD, tx_buffer, numBytes) != numBytes) {
        fprintf(stderr, "Failed to write all I2C data\n");
        return 1;
    }

    return 0;
}

/*********************************************************
 * @Name:   ArtyI2CRead()
 * @pre:    Assumes that the I2C channel has already been opened
 * @param:  uint8_t channel     The I2C channel to open
 *          uint8_t slaveAdx    The I2C slave address to read from
 *          uint8_t * rx_buffer The location where read data will be stored
 *          uint8_t numBytes    The number of bytes to read from slaveAdx
 *
 * @return: 0 if all bytes requested are read, 1 otherwise
 *
 * Reads numBytes from the I2C device at slaveAdx into rx_buffer
 * 
 ********************************************************/
int ArtyI2CRead(uint8_t channel, uint8_t slaveAdx, uint8_t * rx_buffer, uint8_t numBytes) {
    if(NULL == i2cDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "An I2C device does not exist on this channel: %d\n", channel);
        return 1;
    }

    if(ioctl((*i2cDevices[channel]).i2cFD, I2C_SLAVE, slaveAdx) < 0) {
        perror("Failed to register slave on I2C bus: ");
        return 1;
    }

    if(read((*i2cDevices[channel]).i2cFD, rx_buffer, numBytes) != numBytes) {
        fprintf(stderr, "Failed to read all I2C data\n");
        return 1;
    }

    return 0;
}

/*********************************************************
 * @Name:   ArtyI2CCloseMaster()
 * @pre:    Assumes that the I2C channel exists and is currently open
 * @param:  uint8_t channel     The I2C channel to close
 *
 * @return: 0 on success, 1 on failure
 *
 * Closes the I2C channel
 * 
 ********************************************************/
int ArtyI2CCloseMaster(uint8_t channel) {
    if(NULL == i2cDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "An I2C device does not exist on this channel: %d\n", channel);
        return 1;
    }

    close((*i2cDevices[channel]).i2cFD);
    free(i2cDevices[channel]);
    i2cDevices[channel] = NULL;
    return 0;
}


/*********************************************************
 * @Name:   ArtyUARTOpen()
 * @pre:    Assumes that UART hardware exists and has been
 *          loaded into the live device tree
 * @param:  uint8_t channel     The UART channel to open
 *
 * @return: 0 on success, 1 on failure
 *
 * Opens the specified UART hardware at the specified channel
 * The UART channel defaults to whatever baud rate is default
 * for it upon boot. Usually B9600 or B115200
 * 
 ********************************************************/
int ArtyUartOpen(uint8_t channel) {
    if(NULL != uartDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A UART device already exists on this channel: %d\n", channel);
        return 1;
    }

    char port[50];
    sprintf(port, "/dev/ttyS%d", channel);
    uartDevices[channel] = (UARTdata *)malloc(sizeof(UARTdata));
    if(((*uartDevices[channel]).uartFD = open(port, O_RDWR | O_NONBLOCK | O_NOCTTY)) < 0) {
        perror("Failed to open uart port: ");
        free(uartDevices[channel]);
        uartDevices[channel] = NULL;
        return 1;
    }

    return 0;

}

/*********************************************************
 * @Name:   ArtyUartSetBaudRate()
 * @pre:    Assumes that the UART channel has already been opened
 * @param:  uint8_t channel     The UART channel to open
 *          unsigned long baud  the baud rate to set the channel to
 *              There are a limited number of supported baud rates.
 *              They are: {50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800
 *                          2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}
 *
 * @return: 0 on success, 1 on failure
 *
 * Attempts to set the baud rate of the UART channel.
 * if a valid baudrate is passed, sets this rate on the UART port
 * if an invalid baudrate is passed, does nothing, and gives an error message
 * 
 ********************************************************/
int ArtyUartSetBaudRate(uint8_t channel, unsigned long baud) {
    if(NULL == uartDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A UART device does not exist on this channel: %d\n", channel);
        return 1;
    }
    uint8_t flag = 0;
    speed_t formattedBaud;
    speed_t baudArray[] = {B50, B75, B110, B134, B150, B200, B300, B600, B1200, B1800,
                        B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400};
    unsigned long speeds[] = {50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800,
                        2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400};
    
    for(int i = 0; i < (sizeof(speeds) / 4); i++) {
        if(baud == speeds[i]) {
            flag = 1;
            formattedBaud = baudArray[i];
        }
    }
    if(!flag) {
        fprintf(stderr, "The baud rate requested is not supported.\n");
        fprintf(stderr, "The supported baud rates are 50, 75, 110, 134, 150, "
                        "200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, "
                        "38400, 57600, 115200, 230400\n");
        return 1;
    }

    struct termios tty;
    tcgetattr((*uartDevices[channel]).uartFD, &tty);
    int status;
    
    status = cfsetospeed(&tty, formattedBaud);
    if (status < 0) {
        perror("Failed to set output baud rate: ");
        return 1;
    }
    
    status = cfsetispeed(&tty, formattedBaud);
    if (status < 0) {
        perror("Failed to set input baud rate: ");
        return 1;
    }
    
    tcflush((*uartDevices[channel]).uartFD, TCIOFLUSH);
    status = tcsetattr((*uartDevices[channel]).uartFD, TCSANOW, &tty);
    if(status < 0) {
        perror("Failed to set the tty port: ");
        return 1;
    }

    return 0;
}

/*********************************************************
 * @Name:   ArtyUartGetBytesAvailable()
 * @pre:    Assumes that the UART channel has already been opened
 * @param:  uint8_t channel     The UART channel to open
 *          uint8_t * numBytes  location of the where the number
 *                              of available bytes will be stored
 *
 * @return: 0 on success, 1 on failure
 *
 * Checks the UART channel to see if and how many bytes
 * are available for reading. Stores this value at *numBytes
 * 
 ********************************************************/
int ArtyUartGetBytesAvailable(uint8_t channel, uint8_t * numBytes) {
    if(NULL == uartDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A UART device does not exist on this channel: %d\n", channel);
        return 1;
    }

    int bytes = -1;
    ioctl((*uartDevices[channel]).uartFD, FIONREAD, &bytes);
    if(bytes < 0) {
        return 1;
    } else {
        *numBytes = (uint8_t) bytes;
    }
    
    return 0;
}

/*********************************************************
 * @Name:   ArtyUartRead()
 * @pre:    Assumes that the UART channel has already been opened
 * @param:  uint8_t channel         The UART channel to read from
 *          uint8_t numBytes        The number of bytes to attempt to read
 *          uint8_t * rx_buffer     The location to store received data
 *          uint8_t * numBytesRead  Location to store number of bytes actually read at
 *
 * @return: 0 on success, 1 on failure or if less bytes than requested are read
 *
 * Reads as many bytes as possible from the UART channel until
 * numBytes is reached. Will read less than numBytes if less bytes
 * are available. If less than numBytes is available, stores the number
 * of bytes actually read at *numBytesRead
 * 
 ********************************************************/
int ArtyUartRead(uint8_t channel, uint8_t numBytes, uint8_t * rxbuffer, uint8_t * numBytesRead) {
    if(NULL == uartDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A UART device does not exist on this channel: %d\n", channel);
        return 1;
    }
    uint8_t bytesAvailable = -1;
    ArtyUartGetBytesAvailable(channel, &bytesAvailable);
    if(bytesAvailable >= numBytes) {
        int bytesRead = read((*uartDevices[channel]).uartFD, rxbuffer, numBytes);
        *numBytesRead = (uint8_t) bytesRead;
        if(bytesRead != numBytes) {
            return 1;
        }
    } else {
        fprintf(stderr, "Not enough bytes available for UART read: ");
        return 1;
    }

    return 0;
}

/*********************************************************
 * @Name:   ArtyUartWrite()
 * @pre:    Assumes that the UART channel has already been opened
 * @param:  uint8_t channel         The UART channel to write to
 *          uint8_t numBytes        The number of bytes to write
 *          uint8_t * tx_buffer     The location of data to be written
 *
 * @return: 0 on success, 1 on failure or if less bytes than
 *                  than requested are written
 *
 * Writes numBytes from tx_buffer to the UART channel
 * If less bytes than numBytes is written, returns 1
 * 
 ********************************************************/
int ArtyUartWrite(uint8_t channel, uint8_t numBytes, uint8_t * txbuffer) {
    if(NULL == uartDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A UART device does not exist on this channel: %d\n", channel);
        return 1;
    }
    int transmitted = write((*uartDevices[channel]).uartFD, txbuffer, numBytes);
    if(transmitted != numBytes) {
        return 1;
    }
    
    return 0;
}

/*********************************************************
 * @Name:   ArtyUARTClose()
 * @pre:    Assumes that UART channel is currently open
 * @param:  uint8_t channel     The UART channel to close
 *
 * @return: 0 on success, 1 on failure
 *
 * Closes the specified UART channel
 * 
 ********************************************************/
int ArtyUartClose(uint8_t channel) {
    if(NULL == uartDevices[channel] || channel >= MAX_DEVICE_NUM_COMM) {
        fprintf(stderr, "A UART device does not exist on this channel: %d\n", channel);
        return 1;
    }
    
    close((*uartDevices[channel]).uartFD);
    free(uartDevices[channel]);
    uartDevices[channel] = NULL;
    return 0;

}

static uint8_t reverseBits(uint8_t b){
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

