/*
 * fat12.c
 *
 */
#include <stdio.h>

#include "fat12.h"
// Add code specific to fat12 manipulation. Since some of the fat32 structures are the same as 
// fat12, some (all?) of these functions might be used by fat32.

unsigned long Power(int power){
	unsigned long result = 0;
	if(power == 0) return 1;
	if(power == 1) return 256;
	if(power > 4) return 404;   // out of size range;
	else return result + Power(power - 1) * 256;
}

unsigned int getByte(void* diskStart, int start_byte, int length){
	int i;
 	unsigned int result = 0;
 	unsigned char* disk_start = (unsigned char*) diskStart;
 	for(i = 0; i < length; i++){
 		result += disk_start[start_byte + i] * Power(i);
 	}

 	return result;
}


