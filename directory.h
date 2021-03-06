/*
 * directory.h
 *
 * Functions used to deal with directory entries.
 */

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "direntry.h"

// Add any directory specific data structure definitions here along with any prototypes
typedef struct item {
	int dir;
	char name[11];
	unsigned int subdir_num;
	struct item* parent;
}item; 

#endif
