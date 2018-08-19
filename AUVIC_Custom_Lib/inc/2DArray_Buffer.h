/*
 * 2DArray_Buffer.h
 *
 *  Created on: Aug 13, 2018
 *      Author: Poornachander
 */

#ifndef INC_2DARRAY_BUFFER_H_
#define INC_2DARRAY_BUFFER_H_

#include<string.h>
#include "stm32f4xx.h"

#define BUFFER_DATA_LENGTH (8)
#define BUFFER_SIZE (8)

typedef struct Buffer{
	char data[BUFFER_SIZE][BUFFER_DATA_LENGTH];
	uint8_t data_len[BUFFER_SIZE];
	uint8_t idx_to_load; // Stores index where new element should go
	uint8_t idx_to_pop; // Stores index of the next element to remove
	uint8_t size; // Stores the number of elements in the buffer
	uint8_t overflow_cnt; // Stores the number of buffer overflows
}_2DArray_Buffer_t;

//Public functions ------------------------------

extern void Buffer_add(_2DArray_Buffer_t* b, const char* str, uint8_t len); // str must be \0 terminated
extern int Buffer_pop(_2DArray_Buffer_t* b, char* data);
extern int Buffer_size(_2DArray_Buffer_t* b);
extern int Buffer_overflow(_2DArray_Buffer_t* b);
extern void Buffer_init();

//-----------------------------------------------


#endif /* INC_2DARRAY_BUFFER_H_ */
