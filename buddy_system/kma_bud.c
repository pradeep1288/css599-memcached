/***************************************************************************
 *  Title: Kernel Memory Allocator
 * -------------------------------------------------------------------------
 *    Purpose: Kernel memory allocator based on the buddy algorithm
 *    Author: Stefan Birrer
 *    Version: $Revision: 1.2 $
 *    Last Modification: $Date: 2009/10/31 21:28:52 $
 *    File: $RCSfile: kma_bud.c,v $
 *    Copyright: 2004 Northwestern University
 ***************************************************************************/
/***************************************************************************
 *  ChangeLog:
 * -------------------------------------------------------------------------
 *    $Log: kma_bud.c,v $
 *    Revision 1.2  2009/10/31 21:28:52  jot836
 *    This is the current version of KMA project 3.
 *    It includes:
 *    - the most up-to-date handout (F'09)
 *    - updated skeleton including
 *        file-driven test harness,
 *        trace generator script,
 *        support for evaluating efficiency of algorithm (wasted memory),
 *        gnuplot support for plotting allocation and waste,
 *        set of traces for all students to use (including a makefile and README of the settings),
 *    - different version of the testsuite for use on the submission site, including:
 *        scoreboard Python scripts, which posts the top 5 scores on the course webpage
 *
 *    Revision 1.1  2005/10/24 16:07:09  sbirrer
 *    - skeleton
 *
 *    Revision 1.2  2004/11/05 15:45:56  sbirrer
 *    - added size as a parameter to kma_free
 *
 *    Revision 1.1  2004/11/03 23:04:03  sbirrer
 *    - initial version for the kernel memory allocator project
 *
 ***************************************************************************/
#ifdef KMA_BUD
#define __KMA_IMPL__

/************System include***********************************************/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/************Private include**********************************************/
#include "kpage.h"
#include "kma.h"

/************Defines and Typedefs*****************************************/
/*  #defines and typedefs should have their names in all caps.
 *  Global variables begin with g. Global constants with k. Local
 *  variables should be in all lower case. When initializing
 *  structures and arrays, line everything up in neat columns.
 */

/************Global Variables*********************************************/
typedef struct pageHeaderInfo_struct
{
	kpage_t* pageInfo;
	struct pageHeaderInfo_struct* nextPage;
} pageHeaderInfo;

typedef struct bufferStruct
{
	void* header;
	struct bufferStruct* start;
	bool isAllocated;
	int size;
	void* data;
} buffer;

typedef struct
{
	buffer* nextBuffer;
	int numAllocatedBuffers;
	pageHeaderInfo* firstPage;
} freeListInfo;

typedef struct
{
	kpage_t* pageInfo;
	freeListInfo bytes32;
	freeListInfo bytes64;
	freeListInfo bytes128;
	freeListInfo bytes256;
	freeListInfo bytes512;
	freeListInfo bytes1024;
	freeListInfo bytes2048;
	freeListInfo bytes4096;
	freeListInfo bytes8192;
	int numAllocatedPages;
} freeListPointers;

/************Function Prototypes******************************************/

kpage_t* getEntryPoint();
void* getNextBuffer(freeListInfo*);
void getSpaceIfNeeded(freeListInfo*, int);
void addBufferToFreeList(buffer*, freeListInfo*);
void addPageToFreeList(pageHeaderInfo*, freeListInfo*);
void splitLargerList(int);
void splitLargeBufferToSmallBuffer(freeListInfo*, freeListInfo*, int);
void* removeFirstBuffer(freeListInfo*);
void* splitBuffer(buffer*, int);
buffer* getBuddy(buffer*);
void coalesceIfNecessary(buffer*);
freeListInfo* getFreeList(int);
int getBufferSize(int);
void removeBufferFromFreeList(buffer*, freeListInfo*);
int getOrder(int);

/************External Declaration*****************************************/

/**************Implementation***********************************************/

// Entry point into data structures.
static kpage_t* entryPoint = 0;
static int debug = 1;

void*
kma_malloc(kma_size_t size)
{
	if (debug) printf("\nREQUEST %i\n", size);
	if (entryPoint == 0) {
		entryPoint = getEntryPoint();
	}
	
	int adjustedSize = size + sizeof(buffer);
	int bufferSize = getBufferSize(adjustedSize);
	freeListInfo* freeList = getFreeList(bufferSize);
	
	if (freeList == NULL) {
		return NULL;
	}
	
	getSpaceIfNeeded(freeList, bufferSize);
	return getNextBuffer(freeList);
}

void
kma_free(void* ptr, kma_size_t size)
{
	if (debug) printf("\nFREE %i\n", size);
	buffer* aBuffer = (buffer*)(ptr - sizeof(buffer) + sizeof(void*));
	aBuffer->isAllocated = 0;
	freeListInfo* freeList = aBuffer->header;
	addBufferToFreeList(aBuffer, freeList);
	
	coalesceIfNecessary(aBuffer);
}

void coalesceIfNecessary(buffer* aBuffer) {

	if (aBuffer->size == 8192) {
		removeBufferFromFreeList(aBuffer, getFreeList(aBuffer->size));
		if (debug) printf("Coalesced to max size\n");
		freeListPointers* freeLists = (freeListPointers*)entryPoint->ptr;

		pageHeaderInfo* pageHeader = (void*)aBuffer->start - sizeof(pageHeaderInfo);
		if (debug) printf("The start is at %p and the page header is at %p\n", aBuffer->start, pageHeader);

		free_page(pageHeader->pageInfo);
		freeLists->numAllocatedPages--;
		if (freeLists->numAllocatedPages == 0) {
			free_page(entryPoint);
			entryPoint = 0;
		}
		
		return;
	}

	buffer* buddy = getBuddy(aBuffer);
	if (debug) printf("Trying to coalesce a buffer of size %i\n", aBuffer->size);
	if (!buddy->isAllocated && buddy->size == aBuffer->size) {
		freeListInfo* freeList = getFreeList(buddy->size);
		removeBufferFromFreeList(buddy, freeList);
		removeBufferFromFreeList(aBuffer, freeList);
		
		buffer* parent = buddy < aBuffer ? buddy : aBuffer;

		parent->size = parent->size*2;
		addBufferToFreeList(parent, getFreeList(parent->size));
		coalesceIfNecessary(parent);
	}
}

buffer* getBuddy(buffer* aBuffer) {
	buffer* buddy = aBuffer;
	int order = getOrder(aBuffer->size);
	// This will probably need to be changed to int on the tlab - long on 64-bit machines, int on 32-bit
	long buddyAddr = (long)buddy;
	buddyAddr ^= 1 << order;
	buddy = (buffer*)buddyAddr;
	if (debug) printf("Buffer addr is %p, buddy addr is %p, order is %i\n", aBuffer, buddy, order);
	return buddy;
}

kpage_t* getEntryPoint() {
	if (debug) printf("Getting entry point\n");
	kpage_t* entryPoint = get_page();
	freeListPointers* freeLists = (freeListPointers*)entryPoint->ptr;
	
	freeLists->pageInfo = entryPoint;
	
	freeLists->bytes32.nextBuffer = 0;
	freeLists->bytes32.numAllocatedBuffers = 0;
	freeLists->bytes32.firstPage = 0;
	
	freeLists->bytes64.nextBuffer = 0;
	freeLists->bytes64.numAllocatedBuffers = 0;
	freeLists->bytes64.firstPage = 0;
	
	freeLists->bytes128.nextBuffer = 0;
	freeLists->bytes128.numAllocatedBuffers = 0;
	freeLists->bytes128.firstPage = 0;
	
	freeLists->bytes256.nextBuffer = 0;
	freeLists->bytes256.numAllocatedBuffers = 0;
	freeLists->bytes256.firstPage = 0;
	
	freeLists->bytes512.nextBuffer = 0;
	freeLists->bytes512.numAllocatedBuffers = 0;
	freeLists->bytes512.firstPage = 0;
	
	freeLists->bytes1024.nextBuffer = 0;
	freeLists->bytes1024.numAllocatedBuffers = 0;
	freeLists->bytes1024.firstPage = 0;
	
	freeLists->bytes2048.nextBuffer = 0;
	freeLists->bytes2048.numAllocatedBuffers = 0;
	freeLists->bytes2048.firstPage = 0;
	
	freeLists->bytes4096.nextBuffer = 0;
	freeLists->bytes4096.numAllocatedBuffers = 0;
	freeLists->bytes4096.firstPage = 0;
	
	freeLists->bytes8192.nextBuffer = 0;
	freeLists->bytes8192.numAllocatedBuffers = 0;
	freeLists->bytes8192.firstPage = 0;
	
	freeLists->numAllocatedPages = 0;
	
	return entryPoint;
}

void* getNextBuffer(freeListInfo* freeList) {

	freeList->numAllocatedBuffers++;

	buffer* aBuffer = removeFirstBuffer(freeList);
	aBuffer->header = freeList;
	aBuffer->isAllocated = 1;
	if (debug) printf("Returning %p as the result of malloc\n", aBuffer);
	return &(aBuffer->data);
}

void getSpaceIfNeeded(freeListInfo* freeList, int size) {
	if (debug) printf("Checking %i-byte free list\n", size);
	
	if (freeList->nextBuffer == 0) { // If there is no free buffer
		if (debug) printf("Need to split to get a buffer of size %i\n", size);
		splitLargerList(size);
	}
}

void addBufferToFreeList(buffer* aBuffer, freeListInfo* freeList) {
	if (debug) printf("Adding a buffer %p to the free list %p ", aBuffer, freeList);
	aBuffer->header = freeList->nextBuffer;
	if (debug) printf("and setting its header to %p\n", aBuffer->header);
	freeList->nextBuffer = aBuffer;
}

void removeBufferFromFreeList(buffer* aBuffer, freeListInfo* freeList) {
	buffer* curBuffer = freeList->nextBuffer;
	if (curBuffer == 0) return;
	if (curBuffer == aBuffer) {
		freeList->nextBuffer = aBuffer->header;
	}
	
	while (curBuffer->header != 0 && curBuffer->header != aBuffer) {
		curBuffer = curBuffer->header;
	}
	
	if (curBuffer->header == aBuffer) {
		curBuffer->header = aBuffer->header;
	}
}

void addPageToFreeList(pageHeaderInfo* pageHeader, freeListInfo* freeList) {
	pageHeader->nextPage = freeList->firstPage;
	freeList->firstPage = pageHeader;
}

void splitLargerList(int size) {

	freeListInfo* freeList;
	
	if (size != 8192) {
		freeList = getFreeList(size*2);
		freeListInfo* smallFreeList = getFreeList(size);
		if (freeList->nextBuffer == 0) {
			if (debug) printf("Splitting %i into %i\n", size*2, size);
			splitLargerList(size*2);
		}
		splitLargeBufferToSmallBuffer(freeList, smallFreeList, size);
		
		return;
	}
	
	freeList = getFreeList(size);
	if (freeList->nextBuffer == 0) {
		kpage_t* page = get_page();
		freeListPointers* freeLists = (freeListPointers*)entryPoint->ptr;
		
		freeLists->numAllocatedPages++;
		
		pageHeaderInfo* pageHeader = (pageHeaderInfo*)page->ptr;
		pageHeader->pageInfo = page;
		pageHeader->nextPage = 0;

		addPageToFreeList(pageHeader, freeList);
		
		void* pageBegin = page->ptr + sizeof(pageHeaderInfo);
		
		int numBuffers = (page->size - sizeof(pageHeaderInfo)) / size;
		numBuffers = numBuffers == 0 ? 1 : numBuffers;
		
		if (debug) printf("of size %i at %p with %i buffers\n", page->size, pageBegin, numBuffers);
		
		int i;
		buffer* aBuffer = 0;
		for (i = 0; i < numBuffers; i++) {
			aBuffer = (pageBegin + i * size);
			if (debug) printf("Buffer %i starts at %p ", i + 1, aBuffer);
			aBuffer->header = 0;
			aBuffer->start = pageBegin;
			aBuffer->isAllocated = 0;
			aBuffer->size = size;
			if (debug) printf("and points to %p\n", aBuffer->header);
			addBufferToFreeList(aBuffer, freeList);
		}
	}
}

void splitLargeBufferToSmallBuffer(freeListInfo* largeFreeList, freeListInfo* smallFreeList, int size) {
	if (debug) printf("Splitting a %i buffer into two %i buffers\n", size*2, size);
	buffer* largeBuffer = removeFirstBuffer(largeFreeList);
	if (debug) printf("Begin splitting to add to %p...\n", smallFreeList);
	buffer* one = splitBuffer(largeBuffer, size);
	buffer* two = one + size/sizeof(buffer);
	// ghetto hack
	if (size != 4096) addBufferToFreeList(two, smallFreeList);
	addBufferToFreeList(one, smallFreeList);
}


void* removeFirstBuffer(freeListInfo* freeList) {
	if (debug) printf("From free list %p, ", freeList);
	buffer* aBuffer = freeList->nextBuffer;
	if (debug) printf("removing buffer %p and ", aBuffer);
	freeList->nextBuffer = aBuffer->header;
	if (debug) printf("updating the list to point to %p\n", freeList->nextBuffer);
	return aBuffer;
}

void* splitBuffer(buffer* largeBuffer, int size) {
	buffer* one = largeBuffer;
	buffer* two = largeBuffer + size/sizeof(buffer);
	
	one->isAllocated = 0;
	one->size = size;
	two->start = one->start;
	two->isAllocated = 0;
	two->size = size;
	return one;
}

freeListInfo* getFreeList(int size) {
	freeListPointers* freeLists = (freeListPointers*)entryPoint->ptr;
	
	if (size <= 32) {
		return &freeLists->bytes32;
	}
	
	if (size <= 64) {
		return &freeLists->bytes64;
	}
	
	if (size <= 128) {
		return &freeLists->bytes128;
	}
	
	if (size <= 256) {
		return &freeLists->bytes256;
	}
	
	if (size <= 512) {
		return &freeLists->bytes512;
	}
	
	if (size <= 1024) {
		return &freeLists->bytes1024;
	}
	
	if (size <= 2048) {
		return &freeLists->bytes2048;
	}
	
	if (size <= 4096) {
		return &freeLists->bytes4096;
	}
	
	if (size <= 8192) {
		return &freeLists->bytes8192;
	}
	
	return NULL;
}

int getBufferSize(int size) {
	if (size <= 32) {
		return 32;
	}
	
	if (size <= 64) {
		return 64;
	}
	
	if (size <= 128) {
		return 128;
	}
	
	if (size <= 256) {
		return 256;
	}
	
	if (size <= 512) {
		return 512;
	}
	
	if (size <= 1024) {
		return 1024;
	}
	
	if (size <= 2048) {
		return 2048;
	}
	
	if (size <= 4096) {
		return 4096;
	}
	
	if (size <= 8192) {
		return 8192;
	}
	
	return 0;
}

int getOrder(int size) {
	if (size <= 32) {
		return 5;
	}
	
	if (size <= 64) {
		return 6;
	}
	
	if (size <= 128) {
		return 7;
	}
	
	if (size <= 256) {
		return 8;
	}
	
	if (size <= 512) {
		return 9;
	}
	
	if (size <= 1024) {
		return 10;
	}
	
	if (size <= 2048) {
		return 11;
	}
	
	if (size <= 4096) {
		return 12;
	}
	
	if (size <= 8192) {
		return 13;
	}
	
	return 0;
}

#endif // KMA_BUD
