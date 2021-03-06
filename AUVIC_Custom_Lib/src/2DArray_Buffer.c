/*
 * 2DArray_Buffer.c
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */


#include "2DArray_Buffer.h"
#include <stdlib.h>

// Adds an element to the end of the queue
//  - str (the element) must be NULL terminated ('\0') for strcpy
extern void Buffer_add(_2DArray_Buffer_t* b, const char* str, uint8_t len){
	// Insert element
	memcpy(b->data[b->idx_to_load], str, len);
	b->data_len[b->idx_to_load] = len;
	b->idx_to_load++;
	b->idx_to_load %= BUFFER_SIZE;
	b->size++;

	// Check if buffer is full
	if(b->size > BUFFER_SIZE)
	{
		// Remove oldest element
		b->idx_to_pop++;
		b->idx_to_pop %= BUFFER_SIZE;
		b->size--;
		b->overflow_cnt++;
	}
}

// Removes an element from the front of the queue
extern int Buffer_pop(_2DArray_Buffer_t* b, char* data) {
	uint8_t ret = 0;

	// Check if the buffer has anything to pop
	if(b->size)
	{
		// Pop oldest element and store it in data
		memcpy(data, b->data[b->idx_to_pop], b->data_len[b->idx_to_pop]);
		ret = b->data_len[b->idx_to_pop];
		b->idx_to_pop++;
		b->idx_to_pop %= BUFFER_SIZE;
		b->size--;
	}

	return ret;
}

// Reset all variables of the buffer
extern void Buffer_init(_2DArray_Buffer_t* b){
	b->idx_to_load = 0;
	b->idx_to_pop = 0;
	b->size = 0;
	b->overflow_cnt = 0;
}

// Get the size of the buffer
extern int Buffer_size(_2DArray_Buffer_t* b){
	return b->size;
}

// Get the number of overflows that have occurred
extern int Buffer_overflow(_2DArray_Buffer_t* b){
	return b->overflow_cnt;
}

