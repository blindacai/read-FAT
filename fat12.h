/*
 * fat12.h
 *
 * Functions used to deal with the file allocation table.
 */
#ifndef FAT12_H
#define FAT12_H

unsigned int getByte(void* diskStart, int start_byte, int length);

#endif
