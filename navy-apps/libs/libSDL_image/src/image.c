#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

/* Load an image from a filesystem path into a software surface.
   Returns a new SDL surface, or NULL on error.
 */
SDL_Surface* IMG_Load(const char *filename) {
  FILE *fp = fopen(filename, "r");
  printf("filename is %s\n", filename);
  if(!fp) {
    perror("fopen");
    return NULL;
  }
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  unsigned char *buf = (unsigned char *)malloc(size * sizeof(char));
  fseek(fp, 0, SEEK_SET);/* reset position to the start of the file */
  assert(fread(buf, 1, size, fp) != 0);
  SDL_Surface *img_surface = STBIMG_LoadFromMemory(buf, size);
  fclose(fp);
  free(buf);
  return img_surface;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
