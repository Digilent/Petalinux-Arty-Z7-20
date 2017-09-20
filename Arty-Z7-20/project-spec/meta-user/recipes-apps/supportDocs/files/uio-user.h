/************************************************************************
*
************************************************************************/ 

/************************************************************************
*	Author: Mitchell Orsucci
*	
*	This software is offered freely by Digilent Inc.
*
*	Creation Date: July 19, 2017
*	
*
************************************************************************/ 

#ifndef UIO_USER_H
#define UIO_USER_H

#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h> /* for page size */

#define ACCESS_REG(BASE, OFFSET) (*(uint32_t *)(BASE + OFFSET))

#define PAGE_SIZE getpagesize()

/* This is the main data structure used by uio-user.c */
typedef struct UIO {
	int uio_fd;				// File descriptor
	int map_size;			// Size of the area to map
	void * mapPtr;			// ptr to the virtual memory that has been mapped
} UIO;

UIO * UIO_MAP(uint8_t uioNum, uint8_t mapNum);
uint8_t UIO_UNMAP(void * blockToFree);

#endif //UIO_USER_H
