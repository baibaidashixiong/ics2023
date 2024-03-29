#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset; /* file position offset in ramdisk */
  ReadFn read;
  WriteFn write;
  int fd; /* file descriptor */
  size_t open_offset; /* operating position offset in current file */
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write},
  {"/dev/events", 0, 0, events_read, invalid_write},
  {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

void init_fs() {
  AM_GPU_CONFIG_T ev = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].size = ev.width * ev.height * sizeof(size_t); // initialize the size of /dev/fb
}

int fs_open(const char *pathname, int flags, int mode) {/* for sfs, ignore flags and mode */
  for(int i = 0; i < LENGTH(file_table); i++) {
    // printf("file_table name is %s\n",file_table[i].name);
    // printf("path name is %s\n",file_table[i].name);
    if(strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].fd = i;
      file_table[i].open_offset = 0;
      return i;
    }
  }
  Log("file doesn't exit!");
  assert(0);
  return 1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  if(fd < 2){
    Log("read %s", file_table[fd].name);
    return 0;
  }
  // register readFn callback function
  ReadFn readFn = file_table[fd].read;
  if (readFn != NULL) {
    /* for serial and keyboard device, currently offset is 0 */
    return readFn(buf, 0, len);/* call callback function */
  }
  int fs_size = file_table[fd].size;
  int fs_open_offset = file_table[fd].open_offset;
  for(int i = 0; i < LENGTH(file_table); i++) {
    if(file_table[i].fd == fd) {
      /* disk_offset + open_offset as current position */
      ramdisk_read(buf, file_table[i].disk_offset + file_table[i].open_offset, len);

      if((fs_open_offset + len) >= fs_size)
        len = fs_size - fs_open_offset;
      file_table[i].open_offset += len;
      assert(file_table[i].size >= file_table[i].open_offset);
      return len; /* on success, the number of bytes read is returned */
    }
  }
  Log("fs_read failed!");
  return -1; /* on error, -1 is returned */
}

size_t fs_write(int fd, const void *buf, size_t len) {
  int fs_open_offset = file_table[fd].open_offset;
  int fs_size = file_table[fd].size;
  // register writeFn callback function
  WriteFn writeFn = file_table[fd].write;
  if (writeFn != NULL) {
    /* set offset (what is its purpose?)*/
    // printf("open_offset is %d\n", file_table[fd].open_offset);
    file_table[fd].open_offset += len;
    return writeFn(buf, fs_open_offset, len);
  }
  if((fs_open_offset + len) >= fs_size)
    len = fs_size - fs_open_offset;
  ramdisk_write(buf, file_table[fd].disk_offset + fs_open_offset, len);
  /* the file offset is incremented by the number of bytes actually written */
  file_table[fd].open_offset += len;

  assert(file_table[fd].size >= file_table[fd].open_offset);
  return len; /* on success, the number of bytes written is returned */
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  if(fd < 2) {
    Log("lseek %s", file_table[fd].name);
    return -1; /* on error, -1 is returned */
  }
  size_t lseek_offset = 0;
  size_t fs_size = file_table[fd].size;
  switch (whence)
  {
  case SEEK_SET:/* 0 */
    if(offset > fs_size) lseek_offset = fs_size;
    lseek_offset = offset;
    break;
  case SEEK_CUR:
    lseek_offset = file_table[fd].open_offset + offset;
    break;
  case SEEK_END:
    /* The file offset is set to the size of the file plus offset bytes */
    // if(offset + fs_size > fs_size) lseek_offset = fs_size;
    lseek_offset = fs_size + offset;
    break;
  default:
    panic("wrong lseek whence!");
    return -1; /* on error, -1 is returned */
  }
  /* on success, lseek returns the resulting offset location
    as measured in bytes from the beginning of the file */
  file_table[fd].open_offset = lseek_offset;

  return lseek_offset;
}


int fs_close(int fd) {
  file_table[fd].fd = -1;/* set fd to -1 as invalid fd */
  /*
   * always return 0 as success return,
   * for sys doesn't maintain the status of opened file
   */
  return 0;
}
