/*
 * fatfs.c
 *
 * Basic operations on a FAT filesystem.
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "fatfs.h"

/**
 * How to use this program.
 */
#define USAGE "Usage: %s file-system-file\n"

/*
 * Default size of one sector.
 */
#define DEFAULT_SECTOR_SIZE 512

/*
 * Size of one directory entry.
 */
#define DIR_ENTRY_SIZE 32

/*
 * Function to open the file system and map it into memory. 
 */
void * open_filesystem(int argc, char *argv[])
{
    char *filename;
    void *memory; 
    int fd;
    struct stat statBuff;

    if (argc == 2)
    {
	filename = argv[1];
    }
    else
    {
	fprintf(stderr, USAGE, argv[0]);
	exit(1);
    }

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
      char buff[256];
      snprintf(buff, 256, "Could not open %s", filename);
      perror(buff);
      exit(1);
    }
    
    if (fstat(fd, &statBuff)) {
	perror("Stat failed");
	exit(1);
    }
    memory = mmap(0, statBuff.st_size, PROT_READ,  MAP_FILE | MAP_PRIVATE, fd, 0);
    if (memory == NULL) {
      perror("MMAP of file failed");
      exit(1);
    }
    return memory;
}

int round_up(int remainder){
  return (remainder > 0)? 1 : 0;
}

//PARTI (general system info)
void getSystem_info(filesystem_info *fsinfo, void *diskStart){
  fsinfo->diskStart = diskStart;
  fsinfo->sector_size = getByte(diskStart, 11, 2);
  fsinfo->cluster_size = getByte(diskStart, 13, 1);

  unsigned int root_entries = getByte(diskStart, 17, 2);
  if(root_entries == 0){
    fsinfo->fs_type = FAT32;
    fsinfo->rootdir_size = 0;
    fsinfo->sectors_per_fat = getByte(diskStart, 36, 4);
    fsinfo->hidden_sectors = getByte(diskStart, 28, 4);
  }
  else{
    fsinfo->fs_type = FAT12;
    fsinfo->rootdir_size = root_entries;
    fsinfo->sectors_per_fat = getByte(diskStart, 22, 2);
    fsinfo->hidden_sectors = getByte(diskStart, 28, 2);
  }

  fsinfo->reserved_sectors = getByte(diskStart, 14, 2);

  unsigned int byte_for_root = fsinfo->rootdir_size * DIR_ENTRY_SIZE;
  fsinfo->sectors_for_root = byte_for_root / fsinfo->sector_size + round_up(byte_for_root % fsinfo->sector_size);

  fsinfo->fat_offset = fsinfo->reserved_sectors;

  unsigned int fat_copy = getByte(diskStart, 16, 1);
  fsinfo->rootdir_offset = fsinfo->reserved_sectors + fat_copy * fsinfo->sectors_per_fat;

  fsinfo->cluster_offset = fsinfo->rootdir_offset + fsinfo->sectors_for_root;
}


/*
 * This function sets up information about a FAT filesystem that will be used to read from
 * that file system.
 */
filesystem_info *initialize_filesystem_info(void *diskStart)
{
    filesystem_info *fsinfo = (filesystem_info *) malloc(sizeof(filesystem_info));
    
    // ADD CODE 
    getSystem_info(fsinfo, diskStart);

    return fsinfo;
}


