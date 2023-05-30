#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  // int i;
  // int w = 400;  // TODO: get the correct width
  // int h = 300;  // TODO: get the correct height
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < w * h; i ++) fb[i] = i;
  outl(SYNC_ADDR, 1);
}

/* initial canvas */
void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t screen_size = inl(VGACTL_ADDR);
  uint32_t w = screen_size >> 16;
  uint32_t h = screen_size & 0xffff;

  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  /* draw (w,h) rectangle start from the position of (x,y)*/
  /*
   * This method will refresh the entire screen
   *   multiple times in sequence
   */
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  if(!ctl->sync && (w == 0 || h ==0)) return; 
  else outl(SYNC_ADDR, 1);
  
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *pixels = ctl->pixels;
  uint32_t screen_w = inl(VGACTL_ADDR) >> 16;
  /* store one pixel each time */
  for (int i = y; i < y+h; i++) {
    for (int j = x; j < x+w; j++) {
      /*
       * piexel[i][j] indicate the color of point (i+ctl->x, j+ctl->y)
       *  ctl->w indicate the width of draw area present,
       *  screen_w indicate the width of screen 
       */
      fb[screen_w*i+j] = pixels[w*(i-y)+(j-x)];
    }
  }

  /* this method will refresh the entire screen at once 
   */
  // int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  // if (!ctl->sync && (w == 0 || h == 0)) return;
  // uint32_t *pixels = ctl->pixels;
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // uint32_t screen_w = inl(VGACTL_ADDR) >> 16;
  // for (int i = y; i < y+h; i++) {
  //   for (int j = x; j < x+w; j++) {
  //     fb[screen_w*i+j] = pixels[w*(i-y)+(j-x)];
  //   }
  // }
  // if (ctl->sync) {
  //   outl(SYNC_ADDR, 1);
  // }
}

/* what is the purpose of this function? */
void __am_gpu_status(AM_GPU_STATUS_T *status) {
  /* read the status from device address SYNC_ADDR */
  status->ready = (bool)inl(SYNC_ADDR);
}

void __am_gpu_memcpy(AM_GPU_MEMCPY_T *params) {
  uint32_t *src = params->src, *dst = (uint32_t *)(FB_ADDR + params->dest);
  /* sizeof(uint32_t) = 4 */
  for (int i = 0; i < params->size >> 2; i++, src++, dst++) {
    *dst = *src;
  }
  char *c_src = (char *)src, *c_dst = (char *)dst;
  /* copy the rest data */
  for (int i = 0; i < (params->size & 3); i++) {
    c_dst[i] = c_src[i];
  }
}