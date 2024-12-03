#ifndef DISK_H
#define DISK_H
#define DISK_NAME "disk.img"
#include <unistd.h>
#include <stdio.h>

int write_to_disk(void *buffer, size_t size, off_t offset);
size_t read_from_disk(void *buffer, size_t size, off_t offset);
int truncate_at_offset(size_t size, off_t offset);



#endif