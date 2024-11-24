#ifndef DISK_H
#define DISK_H
#define DISK_NAME "disk.img"
#include <unistd.h>
#include <stdio.h>

int write_to_disk(char*buffer, size_t size, off_t offset);
int read_from_disk(char* buffer, size_t size, off_t offset);
int truncate_at_offset(size_t size, off_t offset);



#endif