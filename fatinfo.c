/*
 * Program to print information about a FAT file system.
 */
#include <stdio.h>

#include "fatfs.h"
#include "directory.h"

/*
 * Function to print information about a FAT filesystem (useful for debugging).
 */
void print_filesystem_info(const filesystem_info *fsinfo)
{
    printf ("Sector size: %zu\n", fsinfo->sector_size);
    printf ("Cluster size in sectors: %u\n", fsinfo->cluster_size);
    printf ("Root directory size (nb of entries): %u\n", fsinfo->rootdir_size);

    printf ("Sectors per fat: %u\n", fsinfo->sectors_per_fat);
    printf ("Reserved sectors: %u\n", fsinfo->reserved_sectors);
    printf ("Hidden sectors: %u\n", fsinfo->hidden_sectors);

    printf ("Fat offset in sectors: %u\n", fsinfo->fat_offset);
    printf ("Root directory offset in sectors: %u\n", fsinfo->rootdir_offset);
    printf ("First cluster offset in sectors: %u\n", fsinfo->cluster_offset);
}


int is_valid_byte(void* start){
    return (*(unsigned char*)start == 0xe5)? 0 : 1;
}


void print_name(void* dir_mem, int dir){
    printf("%13c", *(unsigned char*)dir_mem);

    int i, j;
    for(i = 1, j = 0; i < 11; i++){
        if( ((unsigned char*)dir_mem)[i] == 0x20 ){
            if( (j == 0) && (dir == 0) ){
                printf(".");
                j++;
            }
        }
        else printf("%c", ((unsigned char*)dir_mem)[i]);
    }

    printf("\n");   // to be deleted
}


void printItem(void* dir_mem, int dir){
    if( !(is_valid_byte(dir_mem)) ) return;

    if(dir){
        printf("DIR");
        printf("%17u", getByte(dir_mem, 26, 2));   // start cluster
    }
    else{
        printf("FILE");
        printf("%16u", getByte(dir_mem, 26, 2));   // start cluster  
    }
    //printf("%13c\n", *(unsigned char*)dir_mem);
    print_name(dir_mem, dir);

    return;
}


void printAll(filesystem_info *fsinfo, void* dir_mem_arg, void* mem, item* the_parent){
    unsigned char* dir_mem = (unsigned char*) dir_mem_arg;
    unsigned char* mem_start  = (unsigned char*) mem;

    item* self = (item*) malloc(sizeof(item));
    self->parent = the_parent;

    while(getByte(dir_mem, 26, 2) != 0){
        if(getByte(dir_mem, 28, 4) != 0) 
            printItem(dir_mem, 0);

        else{
            printItem(dir_mem, 1);
            printAll(fsinfo, 
                    &mem_start[( fsinfo->cluster_offset + (getByte(dir_mem, 26, 2) - 2) * (fsinfo->cluster_size) ) *
                                (fsinfo->sector_size) + 64],    // may put it in a struct
                    mem,
                    self);
        }
        dir_mem += 32;
    }
    return;
}

/*
 * Main function.
 */
int main(int argc, char *argv[])
{
    void * mem = open_filesystem(argc, argv);
    int i;

    filesystem_info *fsinfo = initialize_filesystem_info(mem);
    print_filesystem_info(fsinfo);
    putchar('\n');

    printf("Type    Start Cluster        Size  Name  -> Cluster Chain\n");
    printf("=================================================================\n");

    // Add call to function to print the directory tree
    void* dir_mem_arg = mem + fsinfo->rootdir_offset * fsinfo->sector_size;
    //printf("DIR_ARG is: %c\n", *(unsigned char*)dir_mem_arg);    // FAT12, at line 289

    printAll(fsinfo, dir_mem_arg, mem, NULL);   // may integrate into one struct later
    
}
