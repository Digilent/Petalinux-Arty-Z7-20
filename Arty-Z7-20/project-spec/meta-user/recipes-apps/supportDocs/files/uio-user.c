/*****************************************************************************
*	Maps the memory associated with the FPGA hardware into virtual memory
* 	and returns a struct through which the user can reference the correct
*	area of virtual memory to control the FPGA hardware
*	
*	uioNum -->	the number of the UIO device (these are zero indexed)
*	mapNum -->	the partition number inside the UIO device (also zero indexed)
*
*	information for available UIO devices can be found in /sys/class/uio
*
*
*	returns a pointer to a UIO struct that contains information about the UIO
*	mapping, but more importantly contains the pointer to the location in virtual
*	memory to map to
*****************************************************************************/
/************************************************************************
*	Author: Mitchell Orsucci
*	
*	This software is offered freely by Digilent Inc.
*
*	Creation Date: July 19, 2017
*	
*
************************************************************************/ 
#include "uio-user.h"

static UIO * uios[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

/*****************************************************************************/
/*!
	Initializes the UIO driver and brings the UIO hardware into virtual memory.


 @param	uioNum is the uio device number -- zero-indexed
 @param	mapNum is the specific map partition inside the UIO device
			if the UIO device contains only one partition, then the 
			map number is 0

 @return	a pointer to a UIO struct, which contains the file descripter
 		of the UIO, the size to map, and a pointer to the location in virtual
 		memory where the UIO driver has been mapped

******************************************************************************/
UIO * UIO_MAP(uint8_t uioNum, uint8_t mapNum) {
	UIO * uio = (UIO *) malloc(sizeof(UIO));
	uio->uio_fd = 0;
	uio->map_size = 0;

	char UIO_INFO[100];
	sprintf(UIO_INFO, "/sys/class/uio/uio%d/maps/map%d/size", uioNum, mapNum);
	
	/* Opening as r+ should set the stream to the beginning of the file */
	/* gets rid of the need to fseek */
	fprintf(stderr, "UIO INFO: %s\n", UIO_INFO);
	FILE * size = fopen(UIO_INFO, "r");
	rewind(size);
	fscanf(size, "%x", &(uio->map_size));
	fclose(size);
	fprintf(stderr, "mapSize: 0x%x\n", uio->map_size);
	if(size == NULL) {
		fprintf(stderr, "Invalid uio# or map#\n");
		fprintf(stderr, "ERROR CODE: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	char UIO_FILE[20];
	sprintf(UIO_FILE, "/dev/uio%d", uioNum);
	
	if((uio->uio_fd = open(UIO_FILE, O_RDWR)) < 0) {
		fprintf(stderr, "Failed to open the UIO driver.\n");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "OFFSET = PAGE_SIZE * mapNum: 0x%x\n", PAGE_SIZE * mapNum);
	uio->mapPtr = mmap(0, uio->map_size, PROT_READ | PROT_WRITE, MAP_SHARED, uio->uio_fd, PAGE_SIZE * mapNum);
	if (uio->mapPtr == -1) {
		fprintf(stderr, "MMAP FAILED\n");
		fprintf(stderr, "ERROR CODE: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	UIO * indexPtr;
	for(int i = 0; i < 10; i++) {
		indexPtr = uios[i];
		if(indexPtr == NULL) {
			uios[i] = uio;
			return uio;
		} else if (i == 9){
			fprintf(stderr, "All UIO devices are currently in use\n");
			fprintf(stderr, "Please disable a UIO device before attempting to use this one\n");
			exit(EXIT_FAILURE);
		}
	}
}


/*****************************************************************************/
/*!
	Disables the UIO driver and frees the memory associated with it

 @param	UIO * uio - a pointer to the UIO struct which represents the UIO device
 		to stop

 @return	returns 1 to let the user know when the unmapping has finishes

******************************************************************************/
uint8_t UIO_UNMAP(void * blockToFree) {
	UIO * arrayItem;
	for(int i = 0; i < 10; i++) {	
		arrayItem = uios[i];
		if ((arrayItem != NULL) && (arrayItem->mapPtr == blockToFree)) {
			munmap(arrayItem->mapPtr, arrayItem->map_size);
			close(arrayItem->uio_fd);
			free(arrayItem);
			fprintf(stderr, "UIO device unmapped successfully\n");
			return 0;
		}
	}
	return -1;
}
