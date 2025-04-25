#ifndef SOURCE_DRIVER_RING_BUFFER_H_
#define SOURCE_DRIVER_RING_BUFFER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef struct sRingBufferDesc *RingBuffer_Handle;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

RingBuffer_Handle Ring_Buffer_Init (size_t buffer_capacity);
bool Ring_Buffer_DeInit (RingBuffer_Handle ring_buffer);
bool Ring_Buffer_IsFull (RingBuffer_Handle ring_buffer);
bool Ring_Buffer_IsEmpty (RingBuffer_Handle ring_buffer);
bool Ring_Buffer_Push (RingBuffer_Handle ring_buffer, uint8_t data);
bool Ring_Buffer_Pop (RingBuffer_Handle ring_buffer, uint8_t *data);

#endif /* SOURCE_DRIVER_RING_BUFFER_H_ */
