#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/disk.h"
#include "../include/utils.h"

int write_to_disk(void *buffer, size_t size, off_t offset) {
  int disk;
  int bytesWritten = 0;
  if ((disk = open(DISK_NAME, O_RDWR, O_RDWR)) < 0) {
    perror(RED "write_to_disk(): could not open disk" RESET);
    return -1;
  }
  if (!buffer) {
    perror(RED "write_to_disk(): buffer is null" RESET);
    close(disk);
    return -2;
  }
  if (size < 0 || size > MAX_FILE_SIZE || offset < 0 ||
      offset > MAX_BLOCKS * BLOCK_SIZE) {
    perror(RED "write_to_disk(): Invalid size or offset" RESET);
    close(disk);
    return -3;
  }

  if ((bytesWritten = pwrite(disk, buffer, size, offset)) < 0) {
    perror(RED "write_to_disk():Error writing file" RESET);
    close(disk);
    return -4;
  }
  //   if (bytesWritten < size) {
  //     perror(RED "write_to_disk():Bytes written is less than required"
  //     RESET);
  //   }
  printf(BLUE"DATA WRITTEN SUCCESSFULLY\n"RESET);
  close(disk);
  return 0;
}

size_t read_from_disk(void *buffer, size_t size, off_t offset) {
  int disk;
  size_t bytesRead = -1;
  if ((disk = open(DISK_NAME, O_RDWR, O_RDWR)) < 0) {
    perror(RED "read_from_disk(): could not open disk" RESET);
    return bytesRead;
  }
  if (!buffer) {
    perror(RED "read_from_disk(): buffer is null" RESET);
    close(disk);
    return bytesRead;
  }
  if (size < 0 || size > MAX_FILE_SIZE || offset < 0 ||
      offset > MAX_BLOCKS * BLOCK_SIZE) {
    perror(RED "read_from_disk(): Invalid size or offset" RESET);
    close(disk);
    return bytesRead;
  }
  if ((bytesRead = pread(disk, buffer, size, offset)) < 0) {
    perror(RED "read_from_disk(): could not read" RESET);
    close(disk);
    return bytesRead;
  }
 
  //   if(bytesRead < size){
  //      perror(RED "read_from_disk(): bytes read are less than size" RESET);
  //   }
  close(disk);
  return bytesRead;
}

int truncate_at_offset(size_t size, off_t offset) {
  int disk;
  int bytesWritten = 0;
  char *nullBuf;
  if ((disk = open(DISK_NAME, O_RDWR, O_RDWR)) < 0) {
    perror(RED "truncate_at_offset(): could not open disk" RESET);
    return -1;
  }
  if (size < 0 || size > MAX_FILE_SIZE || offset < 0 ||
      offset > MAX_BLOCKS * BLOCK_SIZE) {
    perror(RED "truncate_at_offset(): Invalid size or offset" RESET);
    close(disk);
    return -3;
  }
  if ((nullBuf = calloc(1, size)) == NULL) {
    perror(RED "truncate_at_offset(): calloc()");
    close(disk);
    return -4;
  }

  if ((bytesWritten = pwrite(disk, nullBuf, size, offset)) < 0) {
    perror(RED "truncate_at_offset(): error writing disk");
    close(disk);
    return -4;
  }
  close(disk);
  return 0;
}