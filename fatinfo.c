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
    unsigned char* head = (unsigned char*)start;
    if((*head == 0xe5) || (*head == 0x00))
        return 0;
    else
        return 1;
}

void print_name(item* new_item, int is_leaf){
    int i;
    for(i = 0; i < 8; i++){
        if(new_item->name[i] != ' ')
            printf("%c", new_item->name[i]);
    }
    if(new_item->dir && (!is_leaf))
        printf("/");
    else{
        if(!( (new_item->name[8] == ' ') && (new_item->name[9] == ' ') && (new_item->name[10] == ' ') )) {
            printf(".");
            int j;
            for(j = 8; j < 11; j++){
                if(new_item->name[j] != ' ')
                    printf("%c", new_item->name[j]);
            }
        }
    }
}

void print_all_name(item* new_item, int is_leaf){
    if(new_item->parent == NULL)
        print_name(new_item, is_leaf);
    else{
        print_all_name(new_item->parent, 0);
        print_name(new_item, is_leaf);
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
    
    print_all_name(new_item, 1);
}


void get_entry12(unsigned int* entry, int odd) {
    if (odd) {
        *entry = *entry >> 12;
    }
    *entry = *entry & 0x000FFF;
}

unsigned int fat_lookup(filesystem_info *fsinfo, void* fat_start, int fat_entry){
    unsigned int get_byte;
    if(fsinfo->fs_type == FAT12) {
        get_byte = getByte(fat_start, (fat_entry/2) * 3, 3);
        get_entry12( &get_byte, fat_entry % 2 );
    }
    else {
        get_byte = getByte(fat_start, fat_entry * 4, 4);
        get_byte = get_byte & 0x0fffffff;
        
    }
    
    return get_byte;
}


void print_chain(filesystem_info *fsinfo, void* mem, unsigned int fat_entry){
    unsigned char* fat_start = mem + (fsinfo->fat_offset) * (fsinfo->sector_size);
    unsigned int prev = fat_entry;
    unsigned int current = fat_lookup(fsinfo, fat_start, prev);
    
    if((current & 0x0fff) == 0x0fff)
        printf("%d,[END]", prev);
    else{
        while(1){
            unsigned int next = fat_lookup(fsinfo, fat_start, current);
            unsigned int indicator = next + current;
            while(next == current + 1){
                current = next;
                next = fat_lookup(fsinfo, fat_start, current);
            }
            
            if( ((next + current) == indicator) && (current != (prev + 1)) ) {
                printf("%d,", prev);
                if(prev != current)
                    printf("%d,", current);
                else {
                    if((next & 0x0fff) == 0x0fff)
                        break;
                    printf("%d", next);
                }
            }
            
            else if(next != current)
                printf("%d-%d,", prev, current);
            else
                printf("%d,", prev);
            
            if((next & 0x0fff) == 0x0fff)
                break;
            
            prev = next;
            current = next;
        }
        printf("[END]");
    }
}


void printItem(filesystem_info *fsinfo, void* mem, void* dir_mem, item* new_item){
    if( !(is_valid_byte(dir_mem)) ) return;
    
    if(new_item->dir){
        printf("DIR");
        printf("%17u", getByte(dir_mem, 26, 2));   // start cluster
    }
    else{
        printf("FILE");
        printf("%16u", getByte(dir_mem, 26, 2));   // start cluster
    }
    
    printf("%11u", getByte(dir_mem, 28, 4));
    
    printf("  ");
    feed_name(dir_mem, new_item);
    
    printf(" -> ");
    print_chain(fsinfo, mem, getByte(dir_mem, 26, 2));
    
    printf("\n");
    
    return;
}

void printAll(filesystem_info *fsinfo, void* dir_mem_arg, void* mem, item* the_parent){
    unsigned char* dir_mem = (unsigned char*) dir_mem_arg;
    unsigned char* mem_start  = (unsigned char*) mem;
    
    while(getByte(dir_mem, 0, 2) != 0){
        item self;
        self.parent = the_parent;
        self.subdir_num = 2;

        if(fsinfo->fs_type == FAT12){
            if(getByte(dir_mem, 16, 2) != 0) {
                the_parent->subdir_num += 1;
                return;
            }
        }

        if(the_parent != NULL) {
            the_parent->subdir_num += 1; 
        } 
        if(getByte(dir_mem, 26, 2) != 0){
            if(getByte(dir_mem, 28, 4) != 0){
                self.dir = 0;
                printItem(fsinfo, mem, dir_mem, &self);
            }
            else{
                self.dir = 1;
                printItem(fsinfo, mem, dir_mem, &self);
                printAll(fsinfo,
                         &mem_start[( fsinfo->cluster_offset + (getByte(dir_mem, 26, 2) - 2) * (fsinfo->cluster_size) ) *
                                    (fsinfo->sector_size) + 64],
                         mem,
                         &self);
            }
        }

        int after_full = fat_lookup(fsinfo, mem + (fsinfo->fat_offset) * (fsinfo->sector_size), getByte(dir_mem, 26, 2));
        while( (after_full > 0x002) && (after_full < 0xFEF) ){
            if(self.subdir_num > 64){
                self.subdir_num = 0;
                printAll(fsinfo, 
                        &mem_start[ ( fsinfo->cluster_offset + (after_full - 2) * (fsinfo->cluster_size) ) * 
                                    (fsinfo->sector_size) ],
                        mem,
                        &self);
            }
            after_full = fat_lookup(fsinfo, mem + (fsinfo->fat_offset) * (fsinfo->sector_size), after_full);
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
    printAll(fsinfo, dir_mem_arg, mem, NULL);
}