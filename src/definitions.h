#ifndef DEFINITIONS_H
#define	DEFINITIONS_H

#include <stdint.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;

#define BIT_MASK_4 0x0F
#define BIT_MASK_8 0xFF
#define BIT_MASK_16 0xFFFF

#define FLAG_ZERO 0x80
#define FLAG_SUB 0x40
#define FLAG_HALF 0x20
#define FLAG_CARRY 0x10

#define MAX_STRING_SIZE 256

#endif	/* DEFINITIONS_H */

