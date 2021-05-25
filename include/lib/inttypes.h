
/*
 * Copyright (c) 2018-2019 Tim Scheuermann, Julian Holzwarth, Adrian Herrmann
 * 
 * Header for integer types.
 * 
 * The code looks cleaner if we don't need to use char whenever we want a short number
 * even though the number is not to be interpreted as a character in any way.
 * Also, this makes it fully clear at a glance what size our numbers are.
 */


#ifndef INTTYPES_H_
#define INTTYPES_H_


#define uint8_t         unsigned char
#define int8_t          char

#define uint16_t        unsigned short
#define int16_t         short

#define uint32_t        unsigned int
#define int32_t         int

#define uint64_t        unsigned long
#define int64_t         long

#define size_t          unsigned int


#endif /* INTTYPES_H_ */
