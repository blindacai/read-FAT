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


/*void print_name(void* dir_mem, int dir, item* new_item){
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
}*/

void print_name(item* new_item){
    int i;
    for(i = 0; i < 8; i++){
        if(new_item->name[i] != ' ')
            printf("%c", new_item->name[i]);   // the name of the file
    }
    if(new_item->dir)
        printf("/");
    else{
        printf(".");
        int j;
        for(j = 8; j < 11; j++){
            if(new_item->name[j] != ' ')
                printf("%c", new_item->name[j]);      // the extension of the file      
        }
    }
}

void print_all_name(item* new_item){
    if(new_item->parent == NULL)
        print_name(new_item);
    else{
        print_all_name(new_item->parent);
        print_name(new_item);
    }
}


// fill in name and ext. of the struct
void feed_name(void* dir_mem, item* new_item){
    int i;
    for(i = 0; i < 8; i++){
        if( ((unsigned char*)dir_mem) [i] == 0x20 )
            new_item->name[i] = ' ';
        else
            new_item->name[i] = ((unsigned char*)dir_mem)[i];
    }

    int j;
    for(j = 8; j < 11; j++){
        if( ((unsigned char*)dir_mem) [j] == 0x20 )
            new_item->name[j] = ' ';
        else
            new_item->name[j] = ((unsigned char*)dir_mem)[j];        
    }

    print_all_name(new_item);
}


void printItem(void* dir_mem, item* new_item){
    if( !(is_valid_byte(dir_mem)) ) return;

    if(new_item->dir){
        printf("DIR");
        printf("%17u", getByte(dir_mem, 26, 2));   // start cluster
    }
    else{
        printf("FILE");
        printf("%16u", getByte(dir_mem, 26, 2));   // start cluster  
    }
    printf("                        ");
    feed_name(dir_mem, new_item);
    printf("\n");

    return;
}


void printAll(filesystem_info *fsinfo, void* dir_mem_arg, void* mem, item* the_parent){
    unsigned char* dir_mem = (unsigned char*) dir_mem_arg;
    unsigned char* mem_start  = (unsigned char*) mem;


    while(getByte(dir_mem, 26, 2) != 0){
        /*item* self = (item*) malloc(sizeof(item));        // hard to free
        self->parent = the_parent;*/
        item self;
        self.parent = the_parent;

        if(getByte(dir_mem, 28, 4) != 0){           // read size to determine file or dir
            self.dir = 0;
            printItem(dir_mem, &self);              // calll print for the files
        }

        else{
            self.dir = 1;
            printItem(dir_mem, &self);               // call print for the dirs (ASS1)
            
            printAll(fsinfo, 
                    &mem_start[ ( fsinfo->cluster_offset + (getByte(dir_mem, 26, 2) - 2) * (fsinfo->cluster_size) ) *
                                (fsinfo->sector_size) + 64 ],    // may put it in a struct
                    mem,
                    &self);                         // print out all the things in subdirs
        }
        dir_mem += 32;                             // move on at the same level
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
